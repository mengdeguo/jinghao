#ifndef _NETOBJ_CACHE_H_
#define _NETOBJ_CACHE_H_

/* key feature: obj_size, alignment*/
struct netobj_cache
{
    size_t  obj_size;
    size_t  total_num; 
    size_t  free_num;
    void *  free_list;
    void *  buf;
};

#define NETOBJ_CACHE(type,name,total)                   \
    static type name##_buf[total];                  \
    static struct netobj_cache name##_cache = {          \
        .obj_size       = sizeof(type),                 \
        .total_num      = total,                        \
        .free_num       = total,                        \
        .free_list      = NULL,                         \
        .buf            = (void *)name##_buf,           \
    }


int init_netobj_cache(struct netobj_cache * cache);
void * netobj_cache_alloc(struct netobj_cache * cache);
int netobj_cache_free(struct netobj_cache *cache, void * obj);

#endif //_NETOBJ_CACHE_H_
