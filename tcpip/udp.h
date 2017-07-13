#ifndef _UDP_H_
#define _UDP_H_

struct net_udp_hdr {
	uint16_t src_port;
	uint16_t dst_port;
	uint16_t len;
	uint16_t chksum;
} __PACKED;

#define SIZEOF_UDP_HDR (sizeof(struct net_udp_hdr))

void init_udp();
int process_udp_recv(struct sk_buff *skb, struct net_ip_hdr * ip_hdr);
int process_udp_send(struct socket * sock,struct sk_buff * skb,struct in_addr * dst_ip,uint16_t dst_port);
int add_udp_socket(struct socket * sock);
int remove_udp_socket(struct socket * sock);
#endif //_UDP_H_
