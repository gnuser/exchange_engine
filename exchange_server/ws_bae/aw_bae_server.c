#include "aw_bae_server.h"
#include "aw_message.h"
# include "aw_config.h"
# include "aw_server.h"
# include "aw_auth.h"
# include "aw_sign.h"
# include "aw_kline.h"
# include "aw_depth.h"
# include "aw_price.h"
# include "aw_state.h"
# include "aw_today.h"
# include "aw_deals.h"
# include "aw_order.h"
# include "aw_asset.h"
# include <curl/curl.h>

extern struct settings settings;
enum pair_single_type pst = PAIR;

static int get_market_info(const char *market_name,char **sellbuf,char **buybuf,char **sellchinesebuf,char **buychinesebuf)
{
    if(market_name == NULL)
        return -1;
    bool is_find = false;
    for(int i = 0; i < settings.market_num; ++i)
    {
       if(strcmp(market_name,settings.markets[i].market) == 0)
        {
            *sellbuf = strdup(settings.markets[i].sellname);
            *buybuf = strdup(settings.markets[i].buyname);
            *sellchinesebuf = strdup(settings.markets[i].sellchinesename);
            *buychinesebuf = strdup(settings.markets[i].buychinesename);
            is_find = true;
            break;
        }
    }
    if(!is_find)
        return -1;
    return 0;
}

static size_t post_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    sds *reply = userdata;
    *reply = sdscatlen(*reply, ptr, size * nmemb);
    return size * nmemb;
}

json_t *curl_req(json_t* request)
{
    json_t *reply  = NULL;
    json_t *error  = NULL;
    json_t *result = NULL;

    char *request_data = json_dumps(request, 0);
    json_decref(request);

    CURL *curl = curl_easy_init();
    sds reply_str = sdsempty();

    struct curl_slist *chunk = NULL;
    chunk = curl_slist_append(chunk, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    curl_easy_setopt(curl, CURLOPT_URL, settings.accesshttp);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, post_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &reply_str);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, (long)(1000));
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_data);

    CURLcode ret = curl_easy_perform(curl);
    if (ret != CURLE_OK) {
        log_fatal("curl_easy_perform fail: %s", curl_easy_strerror(ret));
        goto cleanup;
    }

    reply = json_loads(reply_str, 0, NULL);
    if (reply == NULL)
    {
        log_fatal("json_loads fail");
        goto cleanup;
    }
    error = json_object_get(reply, "error");
    if (!json_is_null(error)) {
        log_error("get market list fail: %s", reply_str);
        goto cleanup;
    }
    result = json_object_get(reply, "result");
    json_incref(result);
cleanup:
    free(request_data);
    sdsfree(reply_str);
    curl_easy_cleanup(curl);
    curl_slist_free_all(chunk);
    if (reply)
        json_decref(reply);

    return result;
}


static void curl_method_req(json_t *req,uint64_t id,const char* method,const char* market,bool all)
{
    json_t *param = json_array();
    json_object_set_new(req, "id",json_integer(id));
    json_object_set_new(req, "method",json_string(method));

    if(market != NULL) {
        json_array_append_new(param,json_string(market));
    }
    if(!all){
        json_array_append_new(param,json_integer(1));
        json_array_append_new(param,json_string("0"));
    }
	json_object_set_new(req,"params",param);
}

static void market_kline_curl_method_req(json_t *req,uint64_t id,const char* method,const char* market,
        uint64_t start,uint64_t end,uint64_t interval)
{
    json_t *param = json_array();
    json_object_set_new(req, "id",json_integer(id));
    json_object_set_new(req, "method",json_string(method));

    if(market != NULL) {
        json_array_append_new(param,json_string(market));
    }
    json_array_append_new(param,json_integer(start));
    json_array_append_new(param,json_integer(end));
    json_array_append_new(param,json_integer(interval));

    json_object_set_new(req,"params",param);

}

// Dont remember to relese return value
static char *get_rose(const char* price,  const char* today_new_price)
{
    char* rose = (char*)malloc(10);
    double d_rose = 0;
    if(!price || !today_new_price)
    {
        memset(rose,0,10);
        rose[0] = '0';
        return rose;
    }
    if(/*strcmp(price,"0") != 0 && */strcmp(today_new_price,"0") != 0)
        d_rose = ((atof(price)/atof(today_new_price)) * 100) - 100;
    sprintf(rose,"%f",d_rose);
    return rose;
}

// Convert c-str lowercase to uppercase
static char* strtouppr(char* s)
{
	if(s == NULL)
		return s;
	int len = strlen(s), j;
	for (j = 0; j < len; j++) s[j] = toupper(s[j]);
	return s;
}
static char* bae_strtolower(char* s)
{
	if(s == NULL)
		return s;
	int len = strlen(s), j;
	for (j = 0; j < len; j++) s[j] = tolower(s[j]);
	return s;
}
uint64_t get_now_timestamp()
{
    time_t now = time(NULL);
    return (uint64_t)now;
}

//static char *get_chinese_market_name(const char* market_name)
//{
//    assert(market_name);
//    char* ch_chinese_market_name = NULL;
//    for(int i = 0; i < settings.market.count; ++i)
//    {
//        if(strcmp(market_name,json_string_value(json_object_get(settings.market.market_pair[i],"name"))) == 0)
//        {
//            json_t* chinese_name = json_object_get(settings.market.market_pair[i],"chinese_name");
//            ch_chinese_market_name = (char*)json_string_value(chinese_name);
//        }
//    }
//    return ch_chinese_market_name;
//}
static uint64_t get_seconds(const char* time)
{
	assert(time);
	if(strcmp(time,"1min") == 0){
		return 60;
	}else if(strcmp(time,"5min")==0 ){
		return 60 * 5; 
	}else if(strcmp(time,"15min")==0 ){
		return 60 * 15;
	}else if(strcmp(time,"30min")==0 ){
		return 60 * 30;
	}else if(strcmp(time,"60min")==0 ){
		return 60 * 60;
	}else if(strcmp(time,"1day")==0 ){
		return 60 * 60 * 24;
	}else if(strcmp(time,"3day")==0 ){
		return 60 * 60 * 24 * 3;
	}else if(strcmp(time,"1week")==0 ){
		return 60 * 60 * 24 * 7;
	}else{
		return 1;
	}
}

//static int bae_send_json(nw_ses *ses, const json_t *json)
//{
//    char *message_data = json_dumps(json, 0);
//    if (message_data == NULL)
//        return -__LINE__;
//    log_trace("send to: %"PRIu64", size: %zu, message: %s", ses->id, strlen(message_data), message_data);
//    int ret = ws_send_text(ses, message_data);
//    free(message_data);
//    return ret;
//}
//
char* get_market_from_ch(const char* ch)
{
    assert(ch);
    char *ret = (char*)malloc(sizeof(char) * 16);
    memset(ret,0,sizeof(char)*16);
    char *tmp = (char*)malloc(sizeof(char) * (strlen(ch)+1));
    memset(tmp,0,sizeof(char)*(strlen(ch)+1));
    strcpy(tmp,ch);
    char *pos = tmp;
    char *market_pos = NULL; 
    char *method_pos = NULL;  
    char *params_pos = NULL; 
    int str_num = 0;
    while(*pos++ != '\0')
    {
        if(*pos == '.') {
            ++str_num;
            *pos = '\0';
            if(1 == str_num)
                market_pos = ++pos;
            else if(2 == str_num)
                method_pos = ++pos;
            else
                params_pos = ++pos;
        }
    }
    strcpy(ret,market_pos);
    free(tmp);
    return ret;
}

void bae_send_notify_update_kline(const char *ch, json_t *notify)       
{ 
	json_object_set_new(notify,"ch",json_string(ch));
    json_t *ticks = json_array();

    char *pos = (char*)malloc(sizeof(char)*(strlen(ch)+1));
    char *free_pos = pos;
    memset(pos,0,sizeof(char)*(strlen(ch)+1));
    strcpy(pos,ch);
    char* market_name = NULL;
	char *params_pos = NULL; 
	int str_num = 0;

	while(*pos++ != '\0')
	{
		if(*pos == '.') {
			++str_num;
			*pos = '\0';
            if(2 == str_num)
                ++pos;
            if(1 == str_num)
                market_name = ++pos;
			if(3 == str_num)
				params_pos = ++pos;
		}
	}
    strtouppr(market_name); 
    char* sellbuf=NULL;
    char* buybuf = NULL;
    char* sellchinesebuf = NULL;
    char* buychinesebuf = NULL;
    if(get_market_info(market_name,&sellbuf,&buybuf,&sellchinesebuf,&buychinesebuf) < 0 
        || NULL == sellbuf || NULL == buybuf || NULL == sellchinesebuf || NULL == buychinesebuf)
    {
        json_decref(ticks);
        return;
    }
    json_object_set_new(notify, "pairname",json_string(market_name));
    json_object_set_new(notify,"sellshortname",json_string(sellbuf));
    json_object_set_new(notify,"buyshortname",json_string(buybuf));

    char chinese_name[32] = {0};
    strncpy(chinese_name,sellchinesebuf,strlen(sellchinesebuf));
    chinese_name[strlen(sellchinesebuf)] = '/';
    strncpy(chinese_name+strlen(sellchinesebuf)+1,buychinesebuf,strlen(buychinesebuf));

    json_object_set_new(notify,"name",json_string(chinese_name));

    //uint64_t interval = get_seconds(params_pos);
    //uint64_t end = get_now_timestamp();
    //uint64_t start = end - 60 * interval;
    uint64_t start = 100*get_seconds(params_pos);
    uint64_t end = get_now_timestamp();
    uint64_t interval = get_seconds(params_pos);
    json_t *curl_market_kline_req = json_object();
    market_kline_curl_method_req(curl_market_kline_req,100,"market.kline",market_name,start,end,interval);
    json_t *curl_market_kline_result = curl_req(curl_market_kline_req);
    for(int i = 0; i < json_array_size(curl_market_kline_result); ++i) 
    {
        json_t *tick = json_object();

        json_t *unit_kline_result = json_array_get(curl_market_kline_result,i); // array
        for(int j = 0; j < json_array_size(unit_kline_result); ++j)
        {
            uint64_t uint_timestamp = json_integer_value(json_array_get(unit_kline_result,0));
            const char* open = json_string_value(json_array_get(unit_kline_result,1));
            const char* high = json_string_value(json_array_get(unit_kline_result,3));
            const char* amount = json_string_value(json_array_get(unit_kline_result,6));
            const char* low = json_string_value(json_array_get(unit_kline_result,4));
            const char* show = json_string_value(json_array_get(unit_kline_result,2));

            json_object_set_new(tick, "kai",json_string(open));
            json_object_set_new(tick, "high",json_string(high));
            json_object_set_new(tick, "amount",json_string(amount));
            json_object_set_new(tick, "low",json_string(low));
            json_object_set_new(tick, "shou",json_string(show));
            json_object_set_new(tick, "timestamp",json_integer(uint_timestamp));
            json_array_append(ticks,tick);
            json_decref(tick);
        }
    }
    
    json_object_set_new(notify,"tick",ticks);
    json_decref(curl_market_kline_result);
    free(free_pos);
    if(sellbuf != NULL) free(sellbuf);
    if(buybuf != NULL) free(buybuf);
    if(sellchinesebuf != NULL) free(sellchinesebuf);
    if(buychinesebuf != NULL) free(buychinesebuf);
}

// 24h
void bae_send_notify_update_trade(const char *ch,json_t *notify)       
{
    json_t *ticks = json_array();

	json_object_set_new(notify,"ch",json_string(ch));

    json_t * market_depth_req = json_object();
    curl_method_req(market_depth_req,100,"market.depth",NULL,true);
    json_t *curl_market_depth_req = curl_req(market_depth_req);
    int result_size = json_array_size(curl_market_depth_req);
    for(int i = 0; i < result_size; ++i) 
    {
        json_t *tick = json_object();

        json_t *unit_market_depth = json_array_get(curl_market_depth_req,i);

        const char* market = json_string_value(json_object_get(unit_market_depth,"market"));
        const char* ask = json_string_value(json_object_get(unit_market_depth,"ask"));
        const char* bid = json_string_value(json_object_get(unit_market_depth,"bid"));

        json_t * market_status_today_req = json_object();
        curl_method_req(market_status_today_req,100,"market.status_today",market,true);
        json_t *curl_market_status_today_req = curl_req(market_status_today_req);

        const char* total = json_string_value(json_object_get(curl_market_status_today_req,"volume"));
        const char* open = json_string_value(json_object_get(curl_market_status_today_req,"open"));
        const char* price = json_string_value(json_object_get(curl_market_status_today_req,"last"));
        const char* high = json_string_value(json_object_get(curl_market_status_today_req,"high"));
        const char* low = json_string_value(json_object_get(curl_market_status_today_req,"low"));

        const char* today_new_price = get_market_price(market);
        json_object_set_new(tick,"today_first_price",json_string(today_new_price));
        char *rose = get_rose(price,today_new_price);
        char* sellbuf=NULL;
        char* buybuf = NULL;
        char* sellchinesebuf = NULL;
        char* buychinesebuf = NULL;
        if(get_market_info(market,&sellbuf,&buybuf,&sellchinesebuf,&buychinesebuf) < 0 
            || NULL == sellbuf || NULL == buybuf || NULL == sellchinesebuf || NULL == buychinesebuf)
        {
            //json_decref(curl_market_depth_req);
            json_decref(curl_market_status_today_req);
            json_decref(tick);
            continue;
        }
        json_object_set_new(tick, "pairname",json_string(market));
        json_object_set_new(tick,"sellshortname",json_string(sellbuf));
        json_object_set_new(tick,"buyshortname",json_string(buybuf));

        char chinese_name[32] = {0};
        strncpy(chinese_name,sellchinesebuf,strlen(sellchinesebuf));
        chinese_name[strlen(sellchinesebuf)] = '/';
        strncpy(chinese_name+strlen(sellchinesebuf)+1,buychinesebuf,strlen(buychinesebuf));

        json_object_set_new(tick,"name",json_string(chinese_name));

        json_object_set_new(tick, "sellname",json_string(sellchinesebuf));
        json_object_set_new(tick, "buyname",json_string(buychinesebuf));
        json_object_set_new(tick, "total",json_string(total));
        json_object_set_new(tick, "high",json_string(high));
        json_object_set_new(tick, "low",json_string(low));
        json_object_set_new(tick, "price",json_string(price));
        json_object_set_new(tick, "rose",json_string(rose));
        free(rose);
        json_object_set_new(tick, "ask",json_string(ask));
        json_object_set_new(tick, "bid",json_string(bid));

        json_array_append(ticks,tick);
        json_decref(curl_market_status_today_req);
        json_decref(tick);
        if(sellbuf != NULL) free(sellbuf);
        if(buybuf != NULL) free(buybuf);
        if(sellchinesebuf != NULL) free(sellchinesebuf);
        if(buychinesebuf != NULL) free(buychinesebuf);
    }
    json_object_set_new(notify,"tick",ticks);
    json_decref(curl_market_depth_req);
    //json_decref(ticks);
}

void bae_send_notify_update_state_single(const char *ch, json_t *notify)
{
    json_t *ticks = json_array();

    uint64_t ts = get_now_timestamp();
    
    // TODO
    // 全局取ch market_pos
    char *market_pos = get_market_from_ch(ch);

	json_object_set_new(notify,"ch",json_string(ch));
	json_object_set_new(notify,"ts",json_integer(ts));

    json_t * market_depth_req = json_object();
    curl_method_req(market_depth_req,100,"market.depth",NULL,true);
    json_t *curl_market_depth_req = curl_req(market_depth_req);
    int result_size = json_array_size(curl_market_depth_req);
    for(int j = 0; j < result_size; ++j) 
    {
        json_t *tick = json_object();

        json_t *unit_market_depth = json_array_get(curl_market_depth_req,j);

        const char* market = json_string_value(json_object_get(unit_market_depth,"market"));
        const char* ask = json_string_value(json_object_get(unit_market_depth,"ask"));
        const char* bid = json_string_value(json_object_get(unit_market_depth,"bid"));

        if(strcmp(market+(strlen(market)-3), strtouppr(market_pos)) == 0)
        {
            json_t * market_status_today_req = json_object();
            curl_method_req(market_status_today_req,100,"market.status_today",market,true);
            json_t *curl_market_status_today_req = curl_req(market_status_today_req);
            if(curl_market_status_today_req == NULL)
                return;

            const char* total = json_string_value(json_object_get(curl_market_status_today_req,"volume"));
            const char* open = json_string_value(json_object_get(curl_market_status_today_req,"open"));
            const char* price = json_string_value(json_object_get(curl_market_status_today_req,"last"));
            const char* high = json_string_value(json_object_get(curl_market_status_today_req,"high"));
            const char* low = json_string_value(json_object_get(curl_market_status_today_req,"low"));

            const char *today_new_price = get_market_price(market);
            char *rose = get_rose(price,today_new_price);
            
            json_object_set_new(tick, "pairname",json_string(market));
            char sellshortname[7];
            strcpy(sellshortname,market);
            sellshortname[3] = 0;
            json_object_set_new(tick,"sellshortname",json_string(sellshortname));
            char *chinese_name = NULL;
            if(chinese_name != NULL) 
                json_object_set_new(tick,"name",json_string(chinese_name));
            json_object_set_new(tick,"buyshortname",json_string(market + 3));
            json_object_set_new(tick, "total",json_string(total));
            json_object_set_new(tick, "high",json_string(high));
            json_object_set_new(tick, "low",json_string(low));
            json_object_set_new(tick, "price",json_string(price));
            json_object_set_new(tick, "ask",json_string(ask));
            json_object_set_new(tick, "rose",json_string(rose));
            free(rose);
            json_object_set_new(tick, "bid",json_string(bid));

            json_array_append(ticks,tick);
        }
    }
	json_object_set(notify,"tick",ticks);
    free(market_pos);
}

static void curl_method_deals_req(json_t *req,uint64_t id,const char* method,const char* market)
{
    json_t *param = json_array();
    json_object_set_new(req, "id",json_integer(id));
    json_object_set_new(req, "method",json_string(method));

    if(market != NULL) {
        json_array_append_new(param,json_string(market));
    }
    json_array_append_new(param,json_integer(100));
    json_array_append_new(param,json_integer(0));
    json_object_set_new(req,"params",param);
}


void bae_send_notify_update_deals(const char *ch ,json_t *notify)
{
    json_t* ticks = json_array();

	json_object_set_new(notify,"ch",json_string(ch));

    json_t * market_deals = json_object();
    // TODO 
    // 此 market 从全局json中读取
    char* market = get_market_from_ch(ch);
    curl_method_deals_req(market_deals,100,"market.deals",strtouppr(market));
    json_t *results = curl_req(market_deals);

	size_t result_size = json_array_size(results);
    for(size_t i = 0; i < result_size; ++i)
    {
        json_t *tick = json_object();
        json_t *result = json_array_get(results,i);
        const char* price = json_string_value(json_object_get(result,"price"));

        double db_price = atof(price);
        const char* ch_count = json_string_value(json_object_get(result,"amount"));
        double count = atof(ch_count);
        uint64_t update_time = (uint64_t)json_real_value(json_object_get(result,"time"));
        const char* ch_buy_sell = json_string_value(json_object_get(result,"type"));
        uint64_t uint_buy_sell = 0;
        if(strcmp(ch_buy_sell,"sell") == 0)
            uint_buy_sell = 1;

        json_object_set_new(tick, "price", json_real(db_price));
        json_object_set_new(tick, "count", json_real(count));
        json_object_set_new(tick, "updateTime", json_integer(update_time));
        json_object_set_new(tick, "type", json_integer(uint_buy_sell));

        json_array_append(ticks,tick);
        json_decref(tick);
    }
   	json_object_set_new(notify,"tick",ticks);
    json_decref(results);
    free(market);
}

void bae_send_notify_update_state_pair(const char *ch,json_t *notify)
{
    json_t* ticks = json_object();
    
	json_object_set_new(notify,"ch",json_string(ch));
    uint64_t ts = get_now_timestamp();
	json_object_set_new(notify,"ts",json_integer(ts));

    json_t * market_status_today_req = json_object();
    // TODO
    char* market = get_market_from_ch(ch);
    curl_method_req(market_status_today_req,100,"market.status_today",strtouppr(market),true);
    json_t *results = curl_req(market_status_today_req);

    const char* total = json_string_value(json_object_get(results,"volume"));
    const char* high = json_string_value(json_object_get(results,"high"));
    const char* low = json_string_value(json_object_get(results,"low"));
    const char* price = json_string_value(json_object_get(results,"last"));
    const char* open = json_string_value(json_object_get(results,"open"));

    char* ask = NULL;
    char* bid = NULL;

    json_t * req = json_object();
    curl_method_req(req,100,"order.depth",market,false);
    json_t *_curl_req = curl_req(req);
    json_t *asks = json_object_get(_curl_req,"asks");
    json_t *bids = json_object_get(_curl_req,"bids");
    if(json_array_size(asks) > 0){
        ask = (char*)json_string_value(json_array_get(json_array_get(asks,0),0));
    }else
       ask = "0";

    if(json_array_size(bids) > 0){
        bid = (char*)json_string_value(json_array_get(json_array_get(bids,0),0));
    }else
        bid = "0";

    const char *today_new_price = get_market_price(market);
    char *rose = get_rose(price,today_new_price);

    char* sellbuf=NULL;
    char* buybuf = NULL;
    char* sellchinesebuf = NULL;
    char* buychinesebuf = NULL;
    if(get_market_info(market,&sellbuf,&buybuf,&sellchinesebuf,&buychinesebuf) < 0
        || NULL == sellbuf || NULL == buybuf)
    {
        json_decref(results);
        json_decref(_curl_req);
        free(market);
        return;
    }
    bae_strtolower(market);
    json_object_set_new(ticks, "pairname",json_string(market));
    json_object_set_new(ticks,"sellshortname",json_string(sellbuf));
    json_object_set_new(ticks,"buyshortname",json_string(buybuf));
    json_object_set_new(ticks, "total",json_string(total));
    json_object_set_new(ticks, "high",json_string(high));
    json_object_set_new(ticks, "low",json_string(low));
    json_object_set_new(ticks, "price",json_string(price));
    json_object_set_new(ticks, "rose",json_string(rose));
    free(rose);
    json_object_set_new(ticks, "ask",json_string(ask));
    json_object_set_new(ticks, "bid",json_string(bid));

	json_object_set_new(notify,"tick",ticks);
    free(market);
    json_decref(results);
    json_decref(_curl_req);
    if(sellbuf != NULL) free(sellbuf);
    if(buybuf != NULL) free(buybuf);
    if(sellchinesebuf != NULL) free(sellchinesebuf);
    if(buychinesebuf != NULL) free(buychinesebuf);
}

static void curl_depth_req(json_t *req,uint64_t id,const char* market)
{
        json_t *param = json_array();
        json_object_set_new(req, "id",json_integer(id));
        json_object_set_new(req, "method",json_string("order.depth"));

        if(market != NULL) {
                json_array_append_new(param,json_string(market));
        }   
        json_array_append_new(param,json_integer(30));
        json_array_append_new(param,json_string("0"));
        json_object_set_new(req,"params",param);
}


void bae_send_notify_update_depth(const char *ch, json_t *notify)
{

        json_object_set_new(notify,"ch",json_string(ch));

        uint64_t ts = get_now_timestamp();
        json_object_set_new(notify,"ts",json_integer(ts));

        json_t *market_depth_req = json_object();
        char* market = get_market_from_ch(ch);
        curl_depth_req(market_depth_req,100,strtouppr(market));
        json_t *market_depth_result = curl_req(market_depth_req);
        json_object_set_new(notify,"tick",market_depth_result);
        free(market);
}

//void bae_send_notify_update_depth(const char *ch,json_t *params, json_t *notify)
//{
//	size_t param_size = json_array_size(params);
//	json_t* ticks = json_array();
//
//	json_object_set_new(notify,"ch",json_string(ch));
//
//    uint64_t ts = get_now_timestamp();
//	json_object_set_new(notify,"ts",json_integer(ts));
//
//    for(size_t i = 1; i < param_size-1; ++i)
//    {
//        json_t *param = json_array_get(params,i);
//        json_t *tick = json_object();
//        json_t *asks = json_object_get(param,"asks");
//        json_t *bids = json_object_get(param,"bids");
//        json_object_set(tick, "asks", asks);
//        json_object_set(tick, "bids", bids);
//        json_array_append(ticks,tick);
//        json_decref(tick);
//    }
//    json_object_set_new(notify,"tick",ticks);
//}

int bae_parse_param(json_t *in_msg, json_t *out_msg)
{
    if(!json_is_object(in_msg))
        return -1;

	enum sub_type type = SUB;
	json_t *sub = json_object_get(in_msg, "sub");
	json_t *unsub = json_object();
	if(!sub || !json_is_string(sub)){
		type = UNSUB;
		unsub = json_object_get(in_msg, "unsub");
		if(!unsub || !json_is_string(unsub)){
			return -1;
		}
	}

	json_t *id = json_object_get(in_msg, "id");
	if(!id || !json_is_string(id)){
		json_decref(unsub);
		return -1;
	}

	char* ch_id = (char*)json_string_value(id); // id10
	char* ch_id_num = ch_id + 2; // 10
	uint64_t id_num = (uint64_t)atoi(ch_id_num);
	char* ch_sub = NULL;
	if(type == SUB){
		ch_sub = (char*)json_string_value(sub);
	}else{
		ch_sub = (char*)json_string_value(unsub);
	}

	char *pos = ch_sub;
	char *market_pos = NULL; 
	char *method_pos = NULL;  
	char *params_pos = NULL; 
	int str_num = 0;
	while(*pos++ != '\0')
	{
		if(*pos == '.') {
			++str_num;
			*pos = '\0';
			if(1 == str_num)
				market_pos = ++pos;
			else if(2 == str_num)
				method_pos = ++pos;
			else
				params_pos = ++pos;
		}
	}

	strtouppr(market_pos);
	char method[50];
	json_t *params = json_array();
	if(strcmp(method_pos,"kline") == 0){
        if(type == SUB) {
			strcpy(method,"kline.subscribe");
        }
        else
            strcpy(method,"kline.unsubscribe");
		json_array_append_new(params,json_string(market_pos));
		json_array_append_new(params,json_integer(10));
    }else if(strcmp(method_pos,"depth") == 0){
        if(type == SUB)
            strcpy(method,"depth.subscribe");
		else
			strcpy(method,"depth.unsubscribe");

		json_array_append_new(params,json_string(market_pos));
		json_array_append_new(params,json_integer(30));
		json_array_append_new(params,json_string("0.00000001"));
	}else if(strcmp(method_pos,"trade") == 0){
		if(strcmp(params_pos,"detail") == 0){
			if(type == SUB)
				strcpy(method,"deals.subscribe");
			else
				strcpy(method,"deals.unsubscribe");
            json_array_append_new(params,json_string(market_pos));  
		}else{
            // 24h
        	if(type == SUB)
				strcpy(method,"today.subscribe");
			else
				strcpy(method,"today.unsubscribe");
                json_array_append_new(params,json_string("BTCCNY"));  
            //for(int i = 0; i < settings.market.count; ++i) {
            //    json_t* name = json_object_get(settings.market.market_pair[i],"name");
            //    market_pos = (char*)json_string_value(name);
            //    json_array_append_new(params,json_string(market_pos));  
            //}
        }


    }else if(strcmp(method_pos,"coin") == 0){
		if(type == SUB)
			strcpy(method,"state.subscribe");
        else
            strcpy(method,"state.unsubscribe");
        if(pst == PAIR)
            json_array_append_new(params,json_string("BACCNY"));
        else
            json_array_append_new(params,json_string("BTCCNY"));
    }
    json_object_set_new(out_msg,"id",json_integer(id_num));
	json_object_set_new(out_msg,"method",json_string(method));
	json_object_set_new(out_msg,"params", params);

    json_decref(unsub);
    return 0;
}


//void bae_send_notify_update_depth(nw_ses *ses,json_t *params, json_t *notify)
//{
//	size_t param_size = json_array_size(params);
//	json_t* ticks = json_array();
//
//	json_object_set_new(notify,"ch",json_string(ses->param_buf));
//
//    uint64_t ts = get_now_timestamp();
//	json_object_set_new(notify,"ts",json_integer(ts));
//
//    for(size_t i = 1; i < param_size-1; ++i)
//    {
//        json_t *param = json_array_get(params,i);
//        json_t *tick = json_object();
//        json_t *asks = json_object_get(param,"asks");
//        json_t *bids = json_object_get(param,"bids");
//        json_object_set(tick, "asks", asks);
//        json_object_set(tick, "bids", bids);
//        json_array_append(ticks,tick);
//    }
//    json_object_set(notify,"tick",ticks);
//}

//void bae_send_notify_update_deals(nw_ses *ses,json_t *params, json_t *notify)
//{
//	size_t param_size_s = json_array_size(params);
//    json_t* ticks = json_array();
//
//	json_object_set_new(notify,"ch",json_string(ses->param_buf));
//    for(size_t i = 1; i < param_size_s; ++i)
//    {
//        json_t *param_s = json_array_get(params,i);
//        json_t *tick = json_object();
//
//        size_t param_size = json_array_size(param_s);
//        for(size_t j = 0; j < param_size; ++j)
//        {
//            json_t *param = json_array_get(param_s,j);
//            const char* price = json_string_value(json_object_get(param,"price"));
//
//            double db_price = atof(price);
//            const char* ch_count = json_string_value(json_object_get(param,"amount"));
//            uint64_t count = atoi(ch_count);
//            uint64_t update_time = (uint64_t)json_real_value(json_object_get(param,"time"));
//            const char* ch_buy_sell = json_string_value(json_object_get(param,"type"));
//            uint64_t uint_buy_sell = 0;
//            if(strcmp(ch_buy_sell,"sell") == 0)
//                uint_buy_sell = 1;
//
//            json_object_set_new(tick, "price", json_real(db_price));
//            json_object_set_new(tick, "count", json_integer(count));
//            json_object_set_new(tick, "updateTime", json_integer(update_time));
//            json_object_set_new(tick, "type", json_integer(uint_buy_sell));
//
//		    json_array_append(ticks,tick);
//        }
//    }
//	json_object_set(notify,"tick",ticks);
//}

//static void bae_send_notify_update_state_pair(nw_ses *ses,json_t *params, json_t *notify)
//{
//    json_t* ticks = json_object();
//    json_t* param = json_array_get(params,1);
//    
//	json_object_set_new(notify,"ch",json_string(ses->param_buf));
//    uint64_t ts = get_now_timestamp();
//	json_object_set_new(notify,"ts",json_integer(ts));
//
//    const char* pair_name = json_string_value(json_array_get(params,0));
//    const char* total = json_string_value(json_object_get(param,"volume"));
//    const char* high = json_string_value(json_object_get(param,"high"));
//    const char* low = json_string_value(json_object_get(param,"low"));
//    const char* price = json_string_value(json_object_get(param,"last"));
//    const char* open = json_string_value(json_object_get(param,"open"));
//
//    char* ask = NULL;
//    char* bid = NULL;
//
//    json_t * req = json_object();
//    curl_method_req(req,100,"order.depth",pair_name,false);
//    json_t *_curl_req = curl_req(req);
//    json_t *asks = json_object_get(_curl_req,"asks");
//    json_t *bids = json_object_get(_curl_req,"bids");
//    if(json_array_size(asks) > 0){
//        ask = (char*)json_string_value(json_array_get(json_array_get(asks,0),0));
//    }else
//       ask = "0";
//
//    if(json_array_size(bids) > 0){
//        bid = (char*)json_string_value(json_array_get(json_array_get(bids,0),0));
//    }else
//        bid = "0";
//
//    char* rose = get_rose(ask,bid);
//
//    json_object_set_new(ticks, "pairname",json_string(pair_name));
//    char sellshortname[7];
//    strcpy(sellshortname,pair_name);
//    sellshortname[3] = 0;
//    json_object_set_new(ticks,"sellshortname",json_string(sellshortname));
//    json_object_set_new(ticks,"buyshortname",json_string(pair_name + 3));
//    json_object_set_new(ticks, "total",json_string(total));
//    json_object_set_new(ticks, "high",json_string(high));
//    json_object_set_new(ticks, "low",json_string(low));
//    json_object_set_new(ticks, "price",json_string(price));
//    json_object_set_new(ticks, "rose",json_string(rose));
//    free(rose);
//    json_object_set_new(ticks, "ask",json_string(ask));
//    json_object_set_new(ticks, "bid",json_string(bid));
//
//	json_object_set(notify,"tick",ticks);
//}

//static char* get_pair_or_single(const char* ch)
//{
//    assert(ch);
//    char *ret = (char*)malloc(sizeof(char) * 16);
//    memset(ret,0,sizeof(char)*16);
//    char *tmp = (char*)malloc(sizeof(char) * (strlen(ch)+1));
//    memset(tmp,0,sizeof(char)*(strlen(ch)+1));
//    strcpy(tmp,ch);
//    char *pos = tmp;
//    char *market_pos = NULL; 
//    char *method_pos = NULL;  
//    char *params_pos = NULL; 
//    int str_num = 0;
//    while(*pos++ != '\0')
//    {
//        if(*pos == '.') {
//            ++str_num;
//            *pos = '\0';
//            if(1 == str_num)
//                market_pos = ++pos;
//            else if(2 == str_num)
//                method_pos = ++pos;
//            else
//                params_pos = ++pos;
//        }
//    }
//    if(strcmp(method_pos,"coin") == 0)
//    {
//        if(strcmp(params_pos,"pair") == 0)
//            strcpy(ret,"pair");
//        else
//            strcpy(ret,"single");
//    }
//    free(tmp);
//    return ret;
//}

//void bae_send_notify_update_state(const char* ch, json_t *notify)
//{
//    // TODO
//    // 解析ch对pair 和 single 的情况分别做处理 
//    char* tail = get_pair_or_single(ch);
//    if(strcmp(tail,"pair") == 0)
//        bae_send_notify_update_state_pair(ch,notify);
//    else
//        bae_send_notify_update_state_single(ch,notify);
//    free(tail);
//}


