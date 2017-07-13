#ifndef _IP_H_
#define _IP_H_

/* Standard well-defined IP protocols.  */
enum {
    IPPROTO_IP = 0,		    /* Dummy protocol for TCP		*/
    IPPROTO_ICMP = 1,		/* Internet Control Message Protocol	*/
    IPPROTO_IGMP = 2,		/* Internet Group Management Protocol	*/
    IPPROTO_IPIP = 4,		/* IPIP tunnels (older KA9Q tunnels use 94) */
    IPPROTO_TCP = 6,		/* Transmission Control Protocol	*/
    IPPROTO_EGP = 8,		/* Exterior Gateway Protocol		*/
    IPPROTO_PUP = 12,		/* PUP protocol				*/
    IPPROTO_UDP = 17,		/* User Datagram Protocol		*/
    IPPROTO_IDP = 22,		/* XNS IDP protocol			*/
    IPPROTO_DCCP = 33,	    /* Datagram Congestion Control Protocol */
    IPPROTO_RSVP = 46,	    /* RSVP protocol			*/
    IPPROTO_GRE = 47,		/* Cisco GRE tunnels (rfc 1701,1702)	*/
    IPPROTO_IPV6	 = 41,	/* IPv6-in-IPv4 tunnelling		*/
    IPPROTO_ESP = 50,       /* Encapsulation Security Payload protocol */
    IPPROTO_AH = 51,        /* Authentication Header protocol       */
    IPPROTO_BEETPH = 94,	/* IP option pseudo header for BEET */
    IPPROTO_PIM    = 103,	/* Protocol Independent Multicast	*/

    IPPROTO_COMP   = 108,   /* Compression Header protocol */
    IPPROTO_SCTP   = 132,	/* Stream Control Transport Protocol	*/
    IPPROTO_UDPLITE = 136,  /* UDP-Lite (RFC 3828)			*/

    IPPROTO_RAW	 = 255,	    /* Raw IP packets			*/
    IPPROTO_MAX
};

#define IPV4_VERSION    4
#define DEFAULT_IPV4_TTL        64
#define DEFAULT_IPV4_TOS        0   
#define IPV4_ICMP_TOS           0xC0

#define	IPV4_MF		            0x2000	/* more fragments bit			*/
#define	IPV4_DF		            0x4000	/* don't fragment bit			*/
#define	IPV4_FRAGOFF	        0x1fff	/* fragment offset mask			*/


struct net_ip_hdr {
	uint8_t vhl;
	uint8_t tos;
    uint16_t len;
    uint16_t id;
    uint16_t offset;
	uint8_t ttl;
	uint8_t proto;
	uint16_t chksum;
	struct in_addr src_ip;
	struct in_addr dst_ip;
} __PACKED;

#define SIZEOF_IP_HDR (sizeof(struct net_ip_hdr))


#define get_ip_hdr_ver(ip_hdr)  (ip_hdr->vhl >> 4)
#define get_ip_hdr_len(ip_hdr)  ((ip_hdr->vhl & 0x0F) << 2)


#define NET_ICMP_DST_UNREACH  3
#define NET_ICMP_ECHO_REQUEST 8
#define NET_ICMP_ECHO_REPLY   0

struct net_icmp_hdr {
	uint8_t type;
	uint8_t code;
	uint16_t chksum;
} __PACKED;

struct in_addr * get_ip_broadcast_address();
int is_ip_multicast_address(struct in_addr * ip);
int process_ip_recv(struct sk_buff * skb);
int process_icmp_recv(struct sk_buff * skb, struct net_ip_hdr * ip_hdr);
int process_ip_send(struct sk_buff * skb, struct in_addr * dst_ip, uint8_t proto, uint8_t tos, uint8_t ttl);

#endif //_IP_H_
