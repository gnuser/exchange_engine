/*
 * Description: 
 *     History: yang@haipo.me, 2017/04/28, create
 */

# include "aw_config.h"
# include "aw_message.h"
# include "aw_asset.h"
# include "aw_order.h"
# include "aw_rate.h"


static kafka_consumer_t *kafka_orders;
static kafka_consumer_t *kafka_balances;
static kafka_consumer_t *kafka_rates;

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

static int process_rates_message(json_t *msg)
{
    // TODO
    return rates_on_update(msg);
}

static void on_rates_message(sds message, int64_t offset)
{
    log_trace("rates message: %s", message);
    json_t *msg = json_loads(message, 0, NULL);
    init_rate = json_deep_copy(msg);
    if (!msg) {
        log_error("invalid rates message: %s", message);
        return;
    }

    int ret = process_rates_message(msg);
    if (ret < 0) {
        log_error("process_rates_message: %s fail: %d", message, ret);
    }

    json_decref(msg);
}

int init_message(void)
{
    init_rate = NULL;
    is_init_rate = false;
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

    settings.rates.offset = RD_KAFKA_OFFSET_BEGINNING;
    kafka_rates = kafka_consumer_create(&settings.rates, on_rates_message);
    if (kafka_rates == NULL) {
        return -__LINE__;
    }
    return 0;
}

