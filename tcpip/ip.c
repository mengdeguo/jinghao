#include "tcpip.h"

static struct in_addr ip_broadcast_address = {.s_addr = 0xffffffff,};

struct in_addr * get_ip_broadcast_address()
{
    return &ip_broadcast_address;
}

struct in_addr get_ip_lan_broadcast_address(struct in_addr * ip, struct in_addr * netmask)
{
    struct in_addr tmp;
    tmp.s_addr = (~netmask->s_addr) | ip->s_addr;

    return tmp;
}

int is_ip_multicast_address(struct in_addr * ip)
{
    /*
     *ip->s_addr_arr[0] >= 224  && ip->s_addr_arr[0]  <= 239
     * */
    return ip->s_addr_arr[0] == 224;
}

static uint16_t get_ip_hdr_checksum(uint16_t * data, int count) 
{
    uint32_t sum = 0;

    while(count-- > 0) {
        sum += *data++;
    }
    
    sum  = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	return (uint16_t)(~sum & 0xFFFF);
}

int process_ip_send(struct sk_buff * skb, struct in_addr * dst_ip, uint8_t proto, uint8_t tos, uint8_t ttl)
{
    uint16_t payload_len = skb_data_len(skb);

    skb_push(skb,SIZEOF_IP_HDR);

    struct net_ip_hdr * ip_hdr  = (struct net_ip_hdr *)skb_data(skb);

    ip_hdr->vhl = (IPV4_VERSION & 0x0f) << 4 | ((SIZEOF_IP_HDR >> 2) & 0x0f);
    ip_hdr->tos = tos;
    ip_hdr->len = htons(payload_len);
    ip_hdr->offset = htons(IPV4_DF);
    ip_hdr->ttl = ttl;
    ip_hdr->proto = proto;
    ip_hdr->dst_ip.s_addr = dst_ip->s_addr; 

    if(!skb_net_if(skb)) { 
        /* if dst ip is in a same LAN with a nif then use this nif 
        * else use the default_netif
        * */
        skb_net_if(skb) = search_appropriate_nif(dst_ip);
    }

    if(!skb_net_if(skb)) {
        free_skb(skb);
        return NET_NO_VALID_NETIF;
    }

    /* fill src_ip */
    ip_hdr->src_ip.s_addr = skb_net_if(skb)->ip.s_addr; 

    /* calculate checksum */
    ip_hdr->chksum = 0;
    ip_hdr->chksum = get_ip_hdr_checksum((uint16_t *)ip_hdr,SIZEOF_IP_HDR >> 1);

    /*
     * using process_eth_send to send the package
     * */
    return process_eth_send(skb);
}

int process_icmp_recv(struct sk_buff * skb, struct net_ip_hdr * ip_hdr)
{
    struct net_icmp_hdr *icmp_hdr = (struct net_icmp_hdr *)skb_data(skb);

    switch(icmp_hdr->type)
    {
    case NET_ICMP_ECHO_REQUEST:
        /* reuse icmp echo request packet */
        icmp_hdr->type = NET_ICMP_ECHO_REPLY; 

        if(icmp_hdr->chksum >= 0xfff7) {
            icmp_hdr->chksum += 9;
        } else {
            icmp_hdr->chksum += 8;
        }

        struct in_addr dst_ip = ip_hdr->src_ip;

        return process_ip_send(skb,&dst_ip,IPPROTO_ICMP,IPV4_ICMP_TOS,DEFAULT_IPV4_TTL); 
        break;
    default:
        free_skb(skb);
        break;
    }
}

int process_ip_recv(struct sk_buff * skb)
{
    struct net_ip_hdr * ip_hdr = (struct net_ip_hdr * )skb_data(skb);

    /* do some check */
    if(get_ip_hdr_ver(ip_hdr) != IPV4_VERSION) {
        free_skb(skb);
        return NET_UNKNOWN_PKT_ERR;
    }

    /* check dst_ip */
    if(skb->net_if->ip.s_addr != ip_hdr->dst_ip.s_addr) {
        free_skb(skb);
        return NET_PKT_ADDR_ERROR;
    }

    /*checksum*/
    if(get_ip_hdr_checksum((uint16_t *)ip_hdr,get_ip_hdr_len(ip_hdr) >> 1)) {
        free_skb(skb);
        return NET_PKT_CHKSUM_ERR;
    }

    /*length*/
    if(ntohs(ip_hdr->len) + get_ip_hdr_len(ip_hdr) > skb->data_len ) {
        free_skb(skb);
        return NET_PKT_LEN_ERR;
    } 

    skb_data_len(skb) = ntohs(ip_hdr->len) + get_ip_hdr_len(ip_hdr);

    skb_pull(skb,get_ip_hdr_len(ip_hdr));
    switch(ip_hdr->proto)
    {
    case IPPROTO_UDP:
        OS_LOG("receive an udp packet \r\n");
        free_skb(skb);
        break;

    case IPPROTO_TCP:
        OS_LOG("receive an tcp packet \r\n");
        free_skb(skb);
        break;

    case IPPROTO_ICMP:
        process_icmp_recv(skb,ip_hdr);
        break;
    default:
        free_skb(skb);
        break;
    }

}


