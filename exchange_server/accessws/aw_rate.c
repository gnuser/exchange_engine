# include "aw_rate.h"
# include "aw_server.h"


// 用链表来存ses,订阅和取消时向链表中添删除ses,
// 有消息推送来，向链表所有的ses发送rates


static list_t* list_ses;

int rates_on_update(json_t* rates)
{
    list_iter *iter = list_get_iterator(list_ses, LIST_START_HEAD);
    list_node *node = NULL;
    while((node = list_next(iter)) != NULL) {
        return send_notify(node->value, "rates.update", rates);
    }
    list_release_iterator(iter);
    json_decref(rates);
    return -1;
}

static void *node_dup(void *obj)
{
    return sdsnewlen(obj, sdslen(obj));
}

static void node_free(void *obj)
{
    // nw_ses_release(obj);
}

static int node_compare(const void *obj, const void *key)
{
    return memcmp(obj, key, sizeof(nw_ses*));
}

int init_rates(void)
{
    list_type type;
    type.dup = node_dup;
    type.free = node_free;
    type.compare = node_compare;
    list_ses = list_create(&type);
    if(list_ses == NULL)
        return -__LINE__;
    return 0;
}

int rates_subscribe(nw_ses *ses)
{
    if(list_add_node_head(list_ses,ses) == NULL)
        return -__LINE__;
    return 0;
}

int rates_unsubscribe(nw_ses *ses)
{
    list_node *node = list_find(list_ses,ses);
    if(node == NULL)
        return -__LINE__;
    list_del(list_ses,node);
    return 0;
}
