/*
 * Description: 
 *     History: yang@haipo.me, 2017/04/28, create
 */

# include "aw_config.h"
# include "aw_message.h"
# include "aw_asset.h"
# include "aw_order.h"
# include <time.h>

extern struct settings settings;

static bool is_init_json_deals = false;
static json_t *json_deals = NULL;   // {market:[time,price]}
static kafka_consumer_t *kafka_orders;
static kafka_consumer_t *kafka_balances;
static kafka_consumer_t *kafka_deals;


// 今天凌晨12点的时间戳
// static uint64_t today_timestamp() 
// {
//     uint64_t now_time = time(NULL);
//     // 当前时间戳 -（当前时间戳 % 24h）
//     return now_time - (now_time % (60 * 60 * 24)) - 60 * 60 * 8;
// }

static uint64_t today_timestamp() 
{
    time_t t = time(NULL);
    struct tm * tm= localtime(&t);
    tm->tm_hour = 0;
    tm->tm_min = 0;
    tm->tm_sec = 0;
    return  mktime(tm);
}           


const char *get_market_price(const char *market)
{
    if(NULL == market)
        return NULL;
    json_t *deals_value = json_object_get(json_deals,market);
    if(deals_value == NULL)
        return NULL;

    // 如果 json_deals.time < today_timestamp ,默认price为 0
    uint64_t time = json_integer_value(json_array_get(deals_value,0));
    if(time < today_timestamp())
        return "0";

    const char *price = json_string_value(json_array_get(deals_value,1));
    if(price == NULL)
        return NULL;
    return price;
}

static int init_json_deals()
{
    if(json_deals != NULL)
        return 0;
    json_deals = json_object();

    for(int i=0; i<settings.market_num; ++i)
    {
        char market[16] = {0};
        strncpy(market,settings.markets[i].market,strlen(settings.markets[i].market));
        if(market == NULL)
            return -__LINE__;
        json_t *deals_value = json_array();
        json_array_append_new(deals_value, json_integer(0));
        json_array_append_new(deals_value, json_string("0"));
        json_object_set(json_deals,market,deals_value);
    }

    is_init_json_deals = true;

    return 0;
}

static int update_json_deals(const char *market,const char *price,uint64_t set_time)
{
    if(NULL == market || NULL == price)
        return -__LINE__;
    // 判断被更新交易对的时间戳
    // 如果时间戳小于today_timestamp ,在进行更新 
    json_t *deals_value = json_object_get(json_deals,market);
    if(deals_value == NULL)
        return -__LINE__;
    uint64_t time = json_integer_value(json_array_get(deals_value,0));
    if(time >= today_timestamp())
        return 0;
    
    // 更新price
    json_t *new_deals_value = json_array();
    json_array_append_new(new_deals_value,json_integer(set_time));
    json_array_append_new(new_deals_value,json_string(price));
    json_object_set(json_deals,market,new_deals_value);

    return 0;
}

// TODO
static int process_deals_message(json_t *msg)
{
    if(!is_init_json_deals)
        if(init_json_deals() < 0)
            return -__LINE__;
    uint64_t time = (uint64_t)json_real_value(json_array_get(msg, 0));

    const char *market = json_string_value(json_array_get(msg, 1));
    const char *price = json_string_value(json_array_get(msg, 6));
    if(market == NULL || price == NULL)
        return -__LINE__;

    if(time > today_timestamp())
        if(update_json_deals(market,price,time) < 0)
            return -1;
    return 0;
}

// TODO
static void on_deals_message(sds message, int64_t offset)
{
    log_trace("order message: %s", message);
    json_t *msg = json_loads(message, 0, NULL);
    if (!msg) {
        log_error("invalid balance message: %s", message);
        return;
    }

    int ret = process_deals_message(msg);
    if (ret < 0) {
        log_error("process_deals_message: %s fail: %d", message, ret);
    }

    json_decref(msg);
}


static int process_orders_message(json_t *msg)
{
    int event = json_integer_value(json_object_get(msg, "event"));
    if (event == 0)
        return -__LINE__;
    json_t *order = json_object_get(msg, "order");
    if (order == NULL)
        return -__LINE__;
    uint32_t user_id = json_integer_value(json_object_get(order, "user"));
    const char *stock = json_string_value(json_object_get(msg, "stock"));
    const char *money = json_string_value(json_object_get(msg, "money"));
    if (user_id == 0 || stock == NULL || money == NULL)
        return -__LINE__;

    asset_on_update(user_id, stock);
    asset_on_update(user_id, money);
    order_on_update(user_id, event, order);

    return 0;
}

static void on_orders_message(sds message, int64_t offset)
{
    log_trace("order message: %s", message);
    json_t *msg = json_loads(message, 0, NULL);
    if (!msg) {
        log_error("invalid balance message: %s", message);
        return;
    }

    int ret = process_orders_message(msg);
    if (ret < 0) {
        log_error("process_orders_message: %s fail: %d", message, ret);
    }

    json_decref(msg);
}

static int process_balances_message(json_t *msg)
{
    uint32_t user_id = json_integer_value(json_array_get(msg, 1));
    const char *asset = json_string_value(json_array_get(msg, 2));
    if (user_id == 0 || asset == NULL) {
        return -__LINE__;
    }

    asset_on_update(user_id, asset);

    return 0;
}

static void on_balances_message(sds message, int64_t offset)
{
    log_trace("balance message: %s", message);
    json_t *msg = json_loads(message, 0, NULL);
    if (!msg) {
        log_error("invalid balance message: %s", message);
        return;
    }

    int ret = process_balances_message(msg);
    if (ret < 0) {
        log_error("process_balances_message: %s fail: %d", message, ret);
    }

    json_decref(msg);
}

int init_message(void)
{
    settings.orders.offset = RD_KAFKA_OFFSET_END;
    kafka_orders = kafka_consumer_create(&settings.orders, on_orders_message);
    if (kafka_orders == NULL) {
        return -__LINE__;
    }

    settings.balances.offset = RD_KAFKA_OFFSET_END;
    kafka_balances = kafka_consumer_create(&settings.balances, on_balances_message);
    if (kafka_balances == NULL) {
        return -__LINE__;
    }

    settings.deals.offset = RD_KAFKA_OFFSET_BEGINNING;
    kafka_deals = kafka_consumer_create(&settings.deals, on_deals_message);
    if (kafka_deals == NULL) {
        return -__LINE__;
    }

    return 0;
}

