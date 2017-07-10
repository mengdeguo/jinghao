#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include "tcpip.h"

#define MAX_SKB_COUNT   (20)

NETOBJ_CACHE(struct sk_buff, skb,MAX_SKB_COUNT);


int init_skb_allocator()
{
    int ret;

    ret = init_netobj_cache(&skb_cache);

    return ret;
}

static int _skb_free_space(struct sk_buff * skb)
{
    if(skb->end < skb->tail) {
        return -1;
    }

    return (int)(skb->end - skb->tail);
}

struct sk_buff *alloc_skb(size_t size)
{
    struct sk_buff * skb = netobj_cache_alloc(&skb_cache);
   
    if(skb) {

        void * buf = allocate(size);
        if(!buf) {
            netobj_cache_free(&skb_cache,skb);
            return NULL;
        }

        memset(skb,0,sizeof(struct sk_buff));

        INIT_LIST_HEAD(&skb->list);

        skb->tail = skb->head = skb->data = buf;
        skb->end = (uint8_t *)buf + size;
        skb->data_len = 0;
        skb->size = size;
    }

    return skb;
}

void free_skb(struct sk_buff * skb)
{
    if(skb) {
        if(skb->head) {
            deallocate(skb->head);
        }

        netobj_cache_free(&skb_cache,skb);
    }
}

void skb_reserve(struct sk_buff * skb,int len)
{
    if(skb) {
        skb->data += len;
        skb->tail += len;
    }
}

uint8_t *skb_put(struct sk_buff *skb, unsigned int len)
{
	unsigned char *tmp = skb->tail;
	
    skb->tail += len;
	skb->data_len  += len;

    return tmp;
}

uint8_t *skb_push(struct sk_buff *skb, unsigned int len)
{
	skb->data -= len;
	skb->data_len  += len;
	
    return skb->data;
}

uint8_t *skb_pull(struct sk_buff *skb, unsigned int len)
{
    skb->data_len -= len;
    skb->data += len;
	return skb->data;
}

int skb_cpy_pkt(struct sk_buff * skb,uint8_t * data, uint16_t datalen)
{
    int free_space = _skb_free_space(skb);
    
    if(free_space < datalen) {
        return NET_SKB_NO_FREE_SPACE;
    }

    skb->data_len += datalen;
    skb->tail += datalen;

    memcpy(skb->data,data,datalen);

    return datalen;
}


