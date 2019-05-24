#ifndef  AW_BAE_SERVER
#define  AW_BAE_SERVER
# include "aw_config.h"

enum sub_type{
    SUB, 
    UNSUB

};

enum pair_single_type{
    PAIR,
    SINGLE
};

extern json_t *gvl_ch;

void bae_send_notify_update_kline(const char *ch ,json_t *notify) ;
void bae_send_notify_update_depth(const char *ch,json_t *notify) ;
void bae_send_notify_update_deals(const char *ch,json_t *notify) ;
void bae_send_notify_update_state(const char *ch, json_t *notify) ;
void bae_send_notify_update_trade(const char *ch, json_t *notify) ;
void bae_send_notify_update_state_single(const char *ch, json_t *notify);
void bae_send_notify_update_state_pair(const char *ch,json_t *notify);
int bae_parse_param(json_t *in_msg, json_t *out_msg);
uint64_t get_now_timestamp();
json_t *curl_req(json_t* request);
#endif 
