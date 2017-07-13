#include <string.h>
#include "kernel_header.h"
#include "tcpip.h"
#include "platform.h"

static uint8_t eth_broascast_addr[MAC_ADDR_LEN] = {0xff,0xff,0xff,0xff,0xff,0xff};

uint8_t * get_eth_broadcast_addr()
{
    return eth_broascast_addr;
}

int is_eth_broadcast_addr(uint8_t eth_addr[MAC_ADDR_LEN])
{
    return !memcmp(eth_broascast_addr,eth_addr,MAC_ADDR_LEN);
}

/* for IPV6: eth_addr[0] == eth_addr[1] = 0x33*/
int is_eth_multicast_addr(uint8_t *eth_addr)
{
	if (eth_addr[0] == 0x01 && eth_addr[1] == 0x00 && eth_addr[2] == 0x5E) {
		return 1;
	}

    return 0;
}

int process_eth_recv(struct sk_buff* skb)
{
    uint16_t type;

    if(!skb) {
        return NET_PARAMS_ERR;
    }

    struct net_eth_hdr *eth_hdr = (struct net_eth_hdr *)skb_data(skb);
    type = ntohs(eth_hdr->type);

    /* filter packet */
    if (memcmp(skb->net_if->hwaddr,eth_hdr->dst,MAC_ADDR_LEN) && !is_eth_broadcast_addr(eth_hdr->dst) && !is_eth_multicast_addr(eth_hdr->dst)) {
        free_skb(skb);
		return NET_PKT_ADDR_ERROR;
	}

    skb_pull(skb,SIZEOF_ETH_HDR);

    switch(type)
    {
        case ETHTYPE_IP:
            process_ip_recv(skb);
            break;
        case ETHTYPE_ARP:
            process_arp_recv(skb);
            break;
        default:
            free_skb(skb);
            break;
    }

    return 0;
}

int fill_eth_hdr_send(struct sk_buff *skb,uint8_t dst_mac_addr[MAC_ADDR_LEN],uint16_t eth_type)
{

    skb_push(skb,SIZEOF_ETH_HDR);

    struct net_eth_hdr *eth_hdr = (struct net_eth_hdr *)skb_data(skb);

    memcpy(eth_hdr->dst,dst_mac_addr,MAC_ADDR_LEN);
    memcpy(eth_hdr->src,skb_net_if(skb)->hwaddr,MAC_ADDR_LEN);
    eth_hdr->type = htons(eth_type);

    /*this function maybe called from different thread, 
     * so schedule the send operation to tcpip thread 
     */

    /*FIXME: if in the end I move the whole sending process to the tcpip thead
     * here no longer need to do async schedule
     * */

    return post_network_task(skb_net_if(skb)->ops.output,1,skb);
}


