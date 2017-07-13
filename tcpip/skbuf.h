#ifndef _SKBUFF_H_
#define _SKBUFF_H_

/* represent a data package */
struct sk_buff
{
    struct list_head    list;

    /* when received */
    struct net_device   *net_if;

    struct socket       *sock;          /* point to the socket or netconnetion which the packet belongs to*/

    uint8_t             *head;          /* point the head of buf memory area*/
    uint8_t             *end;           /* point to the end the memory area*/
    uint8_t             *data;          /* point to the begin of data content*/
    uint8_t             *tail;          /* point to the tail of data content*/

    uint16_t            data_len;
    uint16_t            size;

    uint32_t            ts;             /* store the time when the package has been received*/
}__ALIGN(sizeof(void *));

#define skb_data(skb)       ((skb)->data)
#define skb_data_len(skb)   ((skb)->data_len)
#define skb_net_if(skb)     ((skb)->net_if)
#define skb_ts(skb)         ((skb)->ts)
#define skb_tail(skb)       ((skb)->tail)

int init_skb_allocator();
struct sk_buff *alloc_skb(size_t size);
void free_skb(struct sk_buff * skb);
void skb_reserve(struct sk_buff * skb,int len);
int skb_cpy_pkt(struct sk_buff * skb,const uint8_t * data, uint16_t datalen);
uint8_t *skb_pull(struct sk_buff *skb, unsigned int len);
uint8_t *skb_push(struct sk_buff *skb, unsigned int len);
uint8_t *skb_put(struct sk_buff *skb, unsigned int len);
#endif //_SKBUFF_H_
