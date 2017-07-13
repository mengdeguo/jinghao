#ifndef _TCPIP_H_
#define _TCPIP_H_

#include <stddef.h>
#include <stdint.h>
#include "list.h"
#include "compiler_attribute.h"

enum net_error
{
    NET_INIT_ERR            = -1000,    
    NET_PARAMS_ERR          = -1001,
    NET_NOMEM_ERR           = -1002,
    NET_SKB_NO_FREE_SPACE   = -1003,
    NET_PKT_ADDR_ERROR      = -1004,
    NET_TIMEOUT_ERR         = -1005,
    NET_ARP_PROTO_ERR       = -1006,
    NET_UNKNOWN_PKT_ERR     = -1007,
    NET_PKT_CHKSUM_ERR      = -1008,
    NET_PKT_LEN_ERR         = -1009,
    NET_NO_VALID_NETIF      = -1010,
};

/* Supported address families. */
#define AF_UNSPEC	0
#define AF_UNIX		1	/* Unix domain sockets 		*/
#define AF_LOCAL	1	/* POSIX name for AF_UNIX	*/
#define AF_INET		2	/* Internet IP Protocol 	*/
#define AF_INET6	10	/* IP version 6			*/
#define AF_RAW      42  /* RAW sockets                  */
#define AF_MAX		43	/* For now.. */

/* Protocol families, same as address families. */
#define PF_UNSPEC	AF_UNSPEC
#define PF_UNIX		AF_UNIX
#define PF_LOCAL	AF_LOCAL
#define PF_INET		AF_INET
#define PF_INET6	AF_INET6
#define PF_RAW      AF_RAW
#define PF_MAX		AF_MAX

#define IP4_ADDR_LOOPBACK  0x7f000001
#define IP4_NETMASK_LOOPBACK 0x7f000000
#define INADDR_ANY          0

typedef uint16_t sa_family_t;
typedef size_t socklen_t;

#define DEFAULT_ETH_MTU (1500)
#define MAC_ADDR_LEN    (6)

/* Internet address. */
struct in_addr {
    union {
        uint32_t s_addr;
        uint8_t  s_addr_arr[4];
    };
};

/** Socket type */
typedef enum {
	SOCK_STREAM = 1,
	SOCK_DGRAM,
}net_sock_type;

#define ex_endian16(x) ((uint16_t) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8)))
#define ex_endian32(x) ((uint32_t) ((((x) >> 24) & 0xff) | \
				   (((x) >> 8) & 0xff00) | \
				   (((x) & 0xff00) << 8) | \
				   (((x) & 0xff) << 24)))

#define ntohs(x) ex_endian16(x)
#define ntohl(x) ex_endian32(x)
#define htons(x) ex_endian16(x)
#define htonl(x) ex_endian32(x)

struct sockaddr {
	sa_family_t	sa_family;	    /* address family, AF_xxx	*/
	char		sa_data[14];	/* 14 bytes of protocol address	*/
};

struct sockaddr_in {
    sa_family_t       sin_family;	/* Address family		*/
    uint16_t          sin_port;	/* Port number			*/
    struct in_addr	sin_addr;	/* Internet address		*/

    /* Pad to size of `struct sockaddr'. */
    unsigned char		__pad[sizeof(struct sockaddr)- sizeof(uint16_t) -
			sizeof(uint16_t) - sizeof(struct in_addr)];
};

struct net_tcp_hdr {
	uint16_t src_port;
	uint16_t dst_port;
	uint8_t seq[4];
	uint8_t ack[4];
	uint8_t offset;
	uint8_t flags;
	uint8_t wnd[2];
	uint16_t chksum;
	uint8_t urg[2];
	uint8_t optdata[0];
} __PACKED;


/* comes from zephyr*/
struct dhcp
{
    int mark;
};


/*********The following code comes from lwip********************/

/** Whether the network interface is 'up'. This is
 * a software flag used to control whether this network
 * interface is enabled and processes traffic.
 * It is set by the startup code (for static IP configuration) or
 * by dhcp/autoip when an address has been assigned.
 */
#define NETIF_FLAG_DOWN         0x00U
#define NETIF_FLAG_UP           0x01U
/** If set, the netif has broadcast capability.
 * Set by the netif driver in its init function. */
#define NETIF_FLAG_BROADCAST    0x02U
/** If set, the netif is one end of a point-to-point connection.
 * Set by the netif driver in its init function. */
#define NETIF_FLAG_POINTTOPOINT 0x04U
/** If set, the interface is configured using DHCP.
 * Set by the DHCP code when starting or stopping DHCP. */
#define NETIF_FLAG_DHCP         0x08U
/** If set, the interface has an active link
 *  (set by the network interface driver).
 * Either set by the netif driver in its init function (if the link
 * is up at that time) or at a later point once the link comes up
 * (if link detection is supported by the hardware). */
#define NETIF_FLAG_LINK_UP      0x10U
/** If set, the netif is an ethernet device using ARP.
 * Set by the netif driver in its init function.
 * Used to check input packet types and use of DHCP. */
#define NETIF_FLAG_ETHARP       0x20U
/** If set, the netif is an ethernet device. It might not use
 * ARP or TCP/IP if it is used for PPPoE only.
 */
#define NETIF_FLAG_ETHERNET     0x40U
/** If set, the netif has IGMP capability.
 * Set by the netif driver in its init function. */
#define NETIF_FLAG_IGMP         0x80U

/*****************************************************************/

struct net_device;
struct sk_buff;

typedef void (*netdev_status_notifier)(struct net_device *);
typedef int (*netdev_output)(struct sk_buff * skb);

struct netdev_ops
{
    /*data output*/
    netdev_output   output;
    /*state change*/
    netdev_status_notifier status_notifier;
};

/* represent a network interface card */
struct net_device
{
    struct list_head list;

    struct mutex    mutex;
    struct in_addr  ip;
    struct in_addr  gw;
    struct in_addr  netmask;

    uint8_t         hwaddr[MAC_ADDR_LEN];
    uint8_t         if_flags;

    struct dhcp     dhcp;

    uint16_t        mtu;
    const char      *name;
    
    void *          drv_private;

    /* data operation interface */
    struct netdev_ops   ops;

};

#include "netobj_cache.h"
#include "skbuf.h"
#include "socket.h"
#include "ethernet.h"
#include "ipaddr.h"
#include "arp.h"
#include "ip.h"
#include "udp.h"

int tcpip_init();
void setup_netdev(struct net_device * dev,const char * name);
void set_netdev_drv_private(struct net_device *dev,void * data);
void * get_netdev_drv_private(struct net_device *dev);
int register_netdev(struct net_device *dev);
struct net_device * search_appropriate_nif(struct in_addr * ip);
struct net_device * find_nif_by_ip(struct in_addr * ip);
void set_netdev_down(struct net_device * dev);
void set_netdev_up(struct net_device * dev);
void set_netdev_status_notifier(struct net_device *dev, netdev_status_notifier notifier);
void set_netdev_output_op(struct net_device *dev,netdev_output output);
void set_default_netif(struct net_device * nif);

extern struct message_queue tcpip_queue;

#define post_network_task(func,para_count, ...)                                             \
        ({                                                                                  \
         int __ret;                                                                         \
         __ret = call_task_on_thread_queue(&tcpip_queue,(general_cross_thread_task)func,    \
                    NULL,para_count, ##__VA_ARGS__);                                        \
         __ret;}) 


#endif //_TCPIP_H_
