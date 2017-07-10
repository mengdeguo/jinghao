#ifndef _ETHERNET_H_
#define _ETHERNET_H_

#define ETHTYPE_RARP	  0x8035U   /* type: Reverse ARP		*/
#define ETHTYPE_ARP       0x0806U
#define ETHTYPE_IP        0x0800U
#define ETHTYPE_VLAN      0x8100U
#define ETHTYPE_PPPOEDISC 0x8863U   /* PPP Over Ethernet Discovery Stage */
#define ETHTYPE_PPPOE     0x8864U   /* PPP Over Ethernet Session Stage */
#define ETHTYPE_EAPOL     0x888e    /* EAPOL */

struct net_eth_hdr {
	uint8_t dst[MAC_ADDR_LEN];
	uint8_t src[MAC_ADDR_LEN];
	uint16_t type;
} __PACKED;

#define SIZEOF_ETH_HDR (sizeof(struct net_eth_hdr))

/* referd to tcp/ip illustrated volume 1*/
struct eth_llc_hdr {
    uint8_t dsap;	            /* destination SAP */
    uint8_t ssap;	            /* source SAP */
    uint8_t llc;		        /* LLC control field */
    uint8_t protid[3];	        /* protocol id */
    uint16_t type;	            /* ether type field */
}__PACKED;

#define SIZEOF_ETH_LLC_HDR  (sizeof(struct eth_llc_hdr))

#endif //_ETHERNET_H_
