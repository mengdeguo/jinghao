#ifndef _ARP_H_
#define _ARP_H_

struct net_arp_hdr {
	uint16_t    hwtype;     /* hardware type			*/
	uint16_t    protocol;   /* protocol type			*/
	uint8_t     hwlen;      /* hardware address length		*/
	uint8_t     protolen;   /* protocol address length		*/
	uint16_t    opcode;     /* arp operation */
	uint8_t     src_mac[MAC_ADDR_LEN];	
	struct in_addr src_ip;
	uint8_t     dst_mac[MAC_ADDR_LEN];
	struct in_addr dst_ip;
}__PACKED;

#define SIZEOF_ARP_HDR (sizeof(struct net_arp_hdr))

#define ARP_REQUEST 1
#define ARP_REPLY   2

#define ARP_HW_ETH	1         /* hwtype : ehternet */

#define ARP_REFRESH_INTERVAL        (20*60*1000)
#define ARP_WAIT_RESPONSE_INTERVAL  (1000)

#define ARP_REQUEST_TRY_LIMIT       (3)

typedef enum 
{
    ARP_ENTRY_PENDING = 0x10,
    ARP_ENTRY_STABLE,
}arp_item_status;

struct arp_item
{
    struct list_head    arp_entry_list;
    struct in_addr      ip;
    uint8_t             mac[MAC_ADDR_LEN];
    struct net_device   *nif;
    struct list_head    waiting_skb_list;
    arp_item_status     status;

    struct timer        timer;
    uint8_t             try_count;
}__ALIGN(sizeof(void *));

int init_arp();
int generate_arp_request(struct net_device * nif,struct in_addr * dst_ip);

#endif //_ARP_H_
