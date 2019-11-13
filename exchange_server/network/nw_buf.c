/*
 * Description: network buf manager
 *     History: yang@haipo.me, 2016/03/16, create
 */

# include <errno.h>
# include <string.h>
# include "nw_buf.h"

# define NW_BUF_POOL_INIT_SIZE 64
# define NW_BUF_POOL_MAX_SIZE  65535
# define NW_CACHE_INIT_SIZE    64
# define NW_CACHE_MAX_SIZE     65535

size_t nw_buf_size(nw_buf *buf)
{
    return buf->wpos - buf->rpos;
}

size_t nw_buf_avail(nw_buf *buf)
{
    return buf->size - buf->wpos;
}

size_t nw_buf_write(nw_buf *buf, const void *data, size_t len)
{
    size_t available = buf->size - buf->wpos;
    size_t wlen = len > available ? available : len;
    memcpy(buf->data + buf->wpos, data, wlen);
    buf->wpos += wlen;
    return wlen;
}

// 考虑未读的数据
void nw_buf_shift(nw_buf *buf)
{
    if (buf->rpos == buf->wpos) { // 如果读写位置相等, 则直接清空
        buf->rpos = buf->wpos = 0;
    } else if (buf->rpos != 0) { // 如果读位置不为0, 则将未读的数据挪到初始位置
        memmove(buf->data, buf->data + buf->rpos, buf->wpos - buf->rpos);
        buf->wpos -= buf->rpos;
        buf->rpos = 0;
    }
}

// size是内存池中单个内存节点大小
nw_buf_pool *nw_buf_pool_create(uint32_t size)
{
    nw_buf_pool *pool = malloc(sizeof(nw_buf_pool));
    if (pool == NULL)
        return NULL;

    pool->size = size; // 内存池的大小
    pool->used = 0;
    pool->free = 0;
    pool->free_total = NW_BUF_POOL_INIT_SIZE; // 默认64个空闲节点
    pool->free_arr = malloc(pool->free_total * sizeof(nw_buf *)); // 存放的是节点指针
    if (pool->free_arr == NULL) {
        free(pool);
        return NULL;
    }

    return pool;
}

/**
    @brief 申请内存池的内存
    @param[in] pool 内存池指针
    @return 返回申请到的内存节点
*/
nw_buf *nw_buf_alloc(nw_buf_pool *pool)
{
    if (pool->free) {
        // 如果内存池有空闲节点,则直接返回最后一个空闲节点
        nw_buf *buf = pool->free_arr[--pool->free];
        buf->size = pool->size;
        buf->rpos = 0;
        buf->wpos = 0;
        buf->next = NULL;
        return buf;
    }

    // 申请一个内存节点, 为什么要多申请一个size的大小?, pool->size才是真正的数据大小
    // 结构体动态数组 https://www.jianshu.com/p/52cd34c95bd9
    nw_buf *buf = malloc(sizeof(nw_buf) + pool->size);
    if (buf == NULL)
        return NULL;
    buf->size = pool->size;
    buf->rpos = 0;
    buf->wpos = 0;
    buf->next = NULL;

    return buf;
}

void nw_buf_free(nw_buf_pool *pool, nw_buf *buf)
{
    if (pool->free < pool->free_total) {    // 如果空闲的节点数小于总数,则将需要释放的节点直接存放到空闲指针列表尾部
        pool->free_arr[pool->free++] = buf;
    } else if (pool->free_total < NW_BUF_POOL_MAX_SIZE) { // 总数小于最大的节点数量
        uint32_t new_free_total = pool->free_total * 2; // 扩大总数为之前的两倍
        void *new_arr = realloc(pool->free_arr, new_free_total * sizeof(nw_buf *)); // 在原有内存地址上,扩展内存
        if (new_arr) {
            pool->free_total = new_free_total;
            pool->free_arr = new_arr;
            pool->free_arr[pool->free++] = buf;
        } else {
            free(buf);
        }
    } else { // 超过限制,则直接释放节点
        free(buf);
    }
}

void nw_buf_pool_release(nw_buf_pool *pool)
{
    for (uint32_t i = 0; i < pool->free; ++i) {
        free(pool->free_arr[i]);
    }
    free(pool->free_arr);
    free(pool);
}

nw_buf_list *nw_buf_list_create(nw_buf_pool *pool, uint32_t limit)
{
    nw_buf_list *list = malloc(sizeof(nw_buf_list));
    if (list == NULL)
        return NULL;
    list->pool = pool;
    list->count = 0;
    list->limit = limit;
    list->head = NULL;
    list->tail = NULL;

    return list;
}

size_t nw_buf_list_write(nw_buf_list *list, const void *data, size_t len)
{
    const void *pos = data;
    size_t left = len;

    // 优先写尾结点
    if (list->tail && nw_buf_avail(list->tail)) { // 如果列表尾指针存在, 并且内存节点还有空间
        size_t ret = nw_buf_write(list->tail, pos, left); // 则存入数据
        left -= ret; // 返回值是已经写入的数据大小,有可能只写入了一部分
        pos += ret;
    }

    while (left) { // 如果还有剩余数据没写入
        if (list->limit && list->count >= list->limit) // 如果列表满了,没有空间可以再写入,则返回已经写入的数据长度
            return len - left;
        nw_buf *buf = nw_buf_alloc(list->pool); // 向内存池申请一个内存节点
        if (buf == NULL) // 如果申请不到
            return len - left;
        if (list->head == NULL) // 设置链表头为新增的内存节点
            list->head = buf;
        if (list->tail != NULL) // 如果尾部不为空,则链接在尾部
            list->tail->next = buf;
        list->tail = buf; // 新申请的节点成为新的尾部
        list->count++; // 增加计数
        size_t ret = nw_buf_write(list->tail, pos, left); // 将剩余的数据写入新申请的内存节点
        left -= ret; // 继续减去已经写入的长度
        pos += ret;  // 挪动数据指针
    }

    return len;
}

// 直接将数据放入内存池链表
size_t nw_buf_list_append(nw_buf_list *list, const void *data, size_t len)
{
    if (list->limit && list->count >= list->limit)
        return 0;
    nw_buf *buf = nw_buf_alloc(list->pool);
    if (buf == NULL)
        return 0;
    if (len > buf->size) { // 如果超过了节点大小, 回收掉新申请的节点
        nw_buf_free(list->pool, buf);
        return 0;
    }
    nw_buf_write(buf, data, len);
    if (list->head == NULL)
        list->head = buf;
    if (list->tail != NULL)
        list->tail->next = buf;
    list->tail = buf;
    list->count++;

    return len;
}

// 每次释放一个
void nw_buf_list_shift(nw_buf_list *list)
{
    // 从头结点开始
    if (list->head) {
        nw_buf *tmp = list->head;
        list->head = tmp->next; // 获取下一个节点,将头指针也后移
        if (list->head == NULL) { // 如果已经到了末尾, 则将尾指针也置空
            list->tail = NULL;
        }
        list->count--;
        nw_buf_free(list->pool, tmp);
    }
}

// 全部释放
void nw_buf_list_release(nw_buf_list *list)
{
    nw_buf *curr = list->head;
    nw_buf *next = NULL;
    while (curr) {
        next = curr->next;
        nw_buf_free(list->pool, curr);
        curr = next;
    }
    free(list);
}

nw_cache *nw_cache_create(uint32_t size)
{
    nw_cache *cache = malloc(sizeof(nw_cache));
    if (cache == NULL)
        return NULL;

    cache->size = size;
    cache->used = 0;
    cache->free = 0;
    cache->free_total = NW_CACHE_INIT_SIZE;
    cache->free_arr = malloc(cache->free_total * sizeof(void *));
    if (cache->free_arr == NULL) {
        free(cache);
        return NULL;
    }

    return cache;
}

void *nw_cache_alloc(nw_cache *cache)
{
    if (cache->free)
        return cache->free_arr[--cache->free];
    return malloc(cache->size);
}

void nw_cache_free(nw_cache *cache, void *obj)
{
    if (cache->free < cache->free_total) {
        cache->free_arr[cache->free++] = obj;
    } else if (cache->free_total < NW_CACHE_MAX_SIZE) {
        uint32_t new_free_total = cache->free_total * 2;
        void *new_arr = realloc(cache->free_arr, new_free_total * sizeof(void *));
        if (new_arr) {
            cache->free_total = new_free_total;
            cache->free_arr = new_arr;
            cache->free_arr[cache->free++] = obj;
        } else {
            free(obj);
        }
    } else {
        free(obj);
    }
}

void nw_cache_release(nw_cache *cache)
{
    for (uint32_t i = 0; i < cache->free; ++i) {
        free(cache->free_arr[i]);
    }
    free(cache->free_arr);
    free(cache);
}

