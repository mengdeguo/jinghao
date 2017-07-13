#include "kernel_header.h"
#include "tcpip.h"

int init_netobj_cache(struct netobj_cache * cache)
{
    if(!cache || !cache->buf || !cache->obj_size) {
        return NET_PARAMS_ERR;
    }

    disable_interrupt();

    char * cache_start = (char *)cache->buf; 
    char * cache_end = (char *) cache->buf + cache->obj_size * cache->total_num;
    char * p = cache_start;
    cache->free_list = NULL;

    for(; p < cache_end; p += cache->obj_size) {
        *(char **)p = cache->free_list;
        cache->free_list = p;
    }

    enable_interrupt();

    return 0;
}

void * netobj_cache_alloc(struct netobj_cache * cache)
{
    if(!cache) {
        return NULL;
    }

    void* ret = NULL;
    disable_interrupt();

    if(cache->free_list) {
        ret = cache->free_list;
        cache->free_list = *(char **)ret;
        cache->free_num--;
    }
    enable_interrupt();

    return ret;
}

int netobj_cache_free(struct netobj_cache *cache, void * obj)
{
    if(!cache || !obj) {
        return NET_PARAMS_ERR;
    }
    
    disable_interrupt();
    
    char * cache_start = (char *)cache->buf; 
    char * cache_end = (char *) cache->buf + cache->obj_size * cache->total_num;

    if((char *)obj < cache_start || (char *)obj >= cache_end) {
        enable_interrupt();
        return NET_PARAMS_ERR;
    }

    *(char **)obj = cache->free_list;
    cache->free_list = obj;
    cache->free_num++;
    
    enable_interrupt();

    return 0;
}
