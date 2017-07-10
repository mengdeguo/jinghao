#include "tcpip.h"

#define ARP_ITEM_COUNT   (10)

struct  
{
    /*use double-linked list instead of rb-tree as arp_cache,
     *for there's some situation we should travers the cache
     *for example : when nif down, remove all the item belongs to this nif
     * */
    struct list_head    head;
    struct mutex        lock;
}arp_cache;

NETOBJ_CACHE(struct arp_item,arp_item,ARP_ITEM_COUNT);

int init_arp()
{
    mutex_init(&arp_cache.lock,RECU_NORMAL);
    INIT_LIST_HEAD(&arp_cache.head);

    return init_netobj_cache(&arp_item_cache);
}

static struct arp_item * alloc_arp_item()
{
    return netobj_cache_alloc(&arp_item_cache);
}

static void free_arp_item(struct arp_item * entry)
{
    netobj_cache_free(&arp_item_cache,entry);
}

struct arp_item * find_arp_entry(struct net_device *nif, struct in_addr *dst_ip)
{
    if(!nif || !dst_ip) {
        return NULL;
    }

    struct list_head *iter = NULL;
    struct arp_item *entry = NULL;

    list_for_each(iter,&arp_cache.head) {

        struct arp_item *tmp = container_of(iter,struct arp_item,arp_entry_list);
        if((tmp->nif == nif) && (tmp->ip.s_addr == dst_ip->s_addr)) {
            entry = tmp;
            break;
        }
    }

    return entry;
}

static void arp_item_timer_handle(void *param)
{
    mutex_lock(&arp_cache.lock);

    struct arp_item *entry = (struct arp_item *)param;
    if(entry->status != ARP_ENTRY_STABLE) {
        if(entry->try_count < ARP_REQUEST_TRY_LIMIT) {
            
            entry->try_count++;
            start_timer(&entry->timer);
            generate_arp_request(entry->nif,&entry->ip);
            
            mutex_unlock(&arp_cache.lock);
            return;
        } 
        
        /* retry too much times ,
         * release waiting_skb_list, release entry item
         * */

        list_del(&entry->arp_entry_list);
        destroy_arp_item(entry);
        free_arp_item(entry);
        
        mutex_unlock(&arp_cache.lock);

        return;
    } 

    /*it's time to refresh the arp item*/
    change_timer(&entry->timer,0,ARP_WAIT_RESPONSE_INTERVAL,ONE_SHOT);
    entry->status = ARP_ENTRY_PENDING;
    entry->try_count = 0;

    start_timer(&entry->timer);
    generate_arp_request(entry->nif,&entry->ip);

    mutex_unlock(&arp_cache.lock);
}

void init_arp_item(struct arp_item *entry,struct net_device * nif,struct in_addr * ip)
{
    memset(entry,0,sizeof(struct arp_item));

    INIT_LIST_HEAD(&entry->arp_entry_list);
    INIT_LIST_HEAD(&entry->waiting_skb_list);

    entry->ip.s_addr = ip->s_addr;
    entry->nif = nif;

    entry->status = ARP_ENTRY_PENDING;

    /* init timer using wa*/
    init_timer(&entry->timer,arp_item_timer_handle,entry,0,
            ARP_WAIT_RESPONSE_INTERVAL,ONE_SHOT);

    entry->try_count = 0;
}

void destroy_arp_item(struct arp_item * entry)
{
    if(is_timer_active(&entry->timer)) {
        stop_timer(&entry->timer);
    }

    if(!list_empty(&entry->waiting_skb_list)) {
        struct list_head *skb_iter,*skb_n;
        list_for_each_safe(skb_iter,skb_n,&entry->waiting_skb_list) {

            struct sk_buff *skb = container_of(skb_iter,struct sk_buff,list);
            free_skb(skb);
            list_del(skb_iter);
        }
    }
}

int generate_arp_request(struct net_device * nif,struct in_addr * dst_ip)
{
    int ret = -1;

    struct sk_buff * skb = alloc_skb(SIZEOF_ARP_HDR + SIZEOF_ETH_HDR + 2); //+2 for the alignment of arp header
    if(skb) {
        skb_net_if(skb) = nif;
        skb_ts(skb) = get_sys_tick();
        skb_reserve(skb,2);

        skb_data(skb) += SIZEOF_ETH_HDR;
        skb_tail(skb) += SIZEOF_ETH_HDR + SIZEOF_ARP_HDR;
        skb_data_len(skb) = SIZEOF_ARP_HDR;

        struct net_arp_hdr * arp_hdr = skb_data(skb);
        
        arp_hdr->hwtype = htons(ARP_HW_ETH);
        arp_hdr->protocol = htons(ETHTYPE_IP);
        arp_hdr->hwlen = MAC_ADDR_LEN;
        arp_hdr->protolen = sizeof(struct in_addr);
        arp_hdr->opcode = htons(ARP_REQUEST);

        memcpy(&arp_hdr->src_ip,&nif->ip,sizeof(struct in_addr));
        memcpy(arp_hdr->src_mac,nif->hwaddr,MAC_ADDR_LEN);

        memcpy(&arp_hdr->dst_ip,dst_ip,sizeof(struct in_addr));
        memcpy(arp_hdr->dst_mac,get_eth_broadcast_addr(),MAC_ADDR_LEN);

        ret = fill_eth_hdr_send(skb,get_eth_broadcast_addr(),ETHTYPE_ARP);
    }

    return ret;
}

void send_gratuitous_arp_request(struct net_device *nif)
{
    generate_arp_request(nif,&nif->ip);
}

/* reuse the incoming sk_buff to send arp reply */
static void generate_arp_reply(struct net_device *nif, struct sk_buff *skb)
{
    struct net_arp_hdr * arp_hdr = skb_data(skb);

    /*maybe someof these field can be resued */
    arp_hdr->hwtype = htons(ARP_HW_ETH);
    arp_hdr->protocol = htons(ETHTYPE_IP);
    arp_hdr->hwlen = MAC_ADDR_LEN;
    arp_hdr->protolen = sizeof(struct in_addr);
    arp_hdr->opcode =  htons(ARP_REPLY);

    memcpy(arp_hdr->dst_mac,arp_hdr->src_mac,MAC_ADDR_LEN);
    memcpy(arp_hdr->src_mac,nif->hwaddr,MAC_ADDR_LEN);

    memcpy(&arp_hdr->dst_ip,&arp_hdr->src_ip,sizeof(struct in_addr));
    memcpy(&arp_hdr->src_ip,&nif->ip,sizeof(struct in_addr));

    skb_data_len(skb) = SIZEOF_ARP_HDR;
}

/*FIXME:
 * IP packet also carry <mac.ip> pair(ip in ip header, mac in eth_hdr),
 * should I call this function when receive an ip packet ?
 * */
void arp_update(struct net_device *nif,struct in_addr * src_ip,uint8_t src_mac[MAC_ADDR_LEN])
{

    mutex_lock(&arp_cache.lock);

    struct arp_item * arp_entry = find_arp_entry(nif,src_ip);
    if(!arp_entry) {

        arp_entry = alloc_arp_item();
        if(!arp_entry) {
            OS_LOG("alloc arp item fail \r\n");
            mutex_unlock(&arp_cache.lock);
            return;
        }

        init_arp_item(arp_entry,nif,src_ip);
        memcpy(arp_entry->mac,src_mac,MAC_ADDR_LEN);
        arp_entry->status = ARP_ENTRY_STABLE;

        change_timer(&arp_entry->timer,0,ARP_REFRESH_INTERVAL,ONE_SHOT);
        start_timer(&arp_entry->timer);
        list_add(&arp_entry->arp_entry_list,&arp_cache.head);

        mutex_unlock(&arp_cache.lock);

        return;
    }

    memcpy(arp_entry->mac,src_mac,MAC_ADDR_LEN);

    stop_timer(&arp_entry->timer);

    if(arp_entry->status == ARP_ENTRY_PENDING) {
        arp_entry->status = ARP_ENTRY_STABLE;
        arp_entry->try_count = 0;
        change_timer(&arp_entry->timer,0,ARP_REFRESH_INTERVAL,ONE_SHOT);
    }

    start_timer(&arp_entry->timer);
    
    /*check if there's any skb waiting for on this arp_item*/
    if(list_empty(&arp_entry->waiting_skb_list)) {
        mutex_unlock(&arp_cache.lock);
        return;
    }

    struct list_head * iter, *n;
    struct sk_buff * skb;
    list_for_each_safe(iter,n,&arp_entry->waiting_skb_list) {
        
        list_del(iter);
        skb = container_of(iter,struct sk_buff,list);

        fill_eth_hdr_send(skb,arp_entry->mac,ETHTYPE_IP);
    }

    mutex_unlock(&arp_cache.lock);
}

int process_arp_recv(struct sk_buff * skb)
{
    int ret;

    if(!skb) {
        return NET_PARAMS_ERR;
    }

    if(skb_data_len(skb) < sizeof(struct net_arp_hdr)) {
        free_skb(skb);
        return NET_ARP_PROTO_ERR;
    }

    struct net_arp_hdr * arp_hdr = skb_data(skb);
    struct net_device  * nif = skb_net_if(skb);

    if(ntohs(arp_hdr->protocol) != ETHTYPE_IP || ntohs(arp_hdr->hwtype) != ARP_HW_ETH) {
        free_skb(skb);
        return NET_UNKNOWN_PKT_ERR;
    }

    /* check whether request my mac? */
    if(memcmp(&arp_hdr->dst_ip,&nif->ip,sizeof(struct in_addr))) {
        free_skb(skb);
        return 0;
    }

    /* Even for the arp request, it also carries a useful arp entry,
     * so we should add this item to arp_cache first
     **/ 
    arp_update(nif,&arp_hdr->src_ip,&arp_hdr->src_mac);

    /* arp request ? if so, give quick response*/
    if(ntohs(arp_hdr->opcode) == ARP_REQUEST)  {

        /* send arp reply, reuse the incoming skb & the skb will be released after send in l2*/
        generate_arp_reply(nif,skb);
        return fill_eth_hdr_send(skb,arp_hdr->dst_mac,ETHTYPE_ARP);
    }

    /*comes here means a arp response */
    free_skb(skb);

    return 0;
}


void netdev_arp_cache_clean(struct net_device * nif) 
{
    struct list_head *iter, *n;
    
    mutex_lock(&arp_cache.lock);

    list_for_each_safe(iter,n,&arp_cache.head) {

        struct arp_item * entry = container_of(iter,struct arp_item,arp_entry_list);
        if(entry->nif == nif) {

            list_del(iter);
            destroy_arp_item(entry);
            free_arp_item(entry);
        }
    }

    mutex_unlock(&arp_cache.lock);
}


/* this function should only be called by ip layer, Assumed that 
 * the incoming skb_data(sdk) point to ip header
 * */
int process_eth_send(struct sk_buff * skb) 
{

    uint8_t dst_mac[6];
    struct net_ip_hdr *ip_hdr = skb_data(skb);
    struct in_addr *dst_ip = &ip_hdr->dst_ip;
    struct net_device * nif = skb_net_if(skb);

    /*check is broadcast or multicast packet */
    if(!memcmp(&dst_ip->s_addr,get_ip_broadcast_address())) {
        return fill_eth_hdr_send(skb,get_eth_broadcast_addr(),ETHTYPE_IP);
    }

    if(is_ip_multicast_address(dst_ip)) {

        dst_mac[0] = 0x01;
        dst_mac[1] = 0x00;
        dst_mac[2] = 0x5E;
        dst_mac[3] = 0x7F & dst_ip->s_addr_arr[1];
        dst_mac[4] = dst_ip->s_addr_arr[2];
        dst_mac[5] = dst_ip->s_addr_arr[3];

        return fill_eth_hdr_send(skb,dst_mac,ETHTYPE_IP);
    }

    /* check if dst ip is in the same LAN
     * if so, using dst ip to find arp entry
     * else using nif.gw to find arp entry
     * */
    if((nif->netmask.s_addr & nif->ip.s_addr) != (dst_ip->s_addr & nif->netmask.s_addr)) {
        dst_ip = &nif->gw;
    }

    mutex_lock(&arp_cache.lock);

    struct arp_item *arp_entry = find_arp_entry(nif,dst_ip);
    if(arp_entry) {
        if(arp_entry->status != ARP_ENTRY_STABLE) {

            list_add_tail(&skb->list,&arp_entry->waiting_skb_list);
            mutex_unlock(&arp_cache.lock);

            return 0;
        }
        
        memcpy(dst_mac,arp_entry->mac,MAC_ADDR_LEN);

        mutex_unlock(&arp_cache.lock);

        /* so this entry is alive */
        return fill_eth_hdr_send(skb,dst_mac,ETHTYPE_IP);
    }

    /*if arp entry not found, allocate a new entry,
     * and put the pkt on the waiting list
     * */

    arp_entry = alloc_arp_item();
    if(!arp_entry) {
        OS_LOG("alloc arp item fail \r\n");
        free_skb(skb);

        mutex_unlock(&arp_cache.lock);
        return NET_NOMEM_ERR;
    }

    init_arp_item(arp_entry,nif,dst_ip);
    list_add_tail(&skb->list,&arp_entry->waiting_skb_list);
    start_timer(&arp_entry->timer);
    list_add(&arp_entry->arp_entry_list,&arp_cache.head);

    mutex_unlock(&arp_cache.lock);

    /*generate arp request*/
    return generate_arp_request(nif,dst_ip);

}


