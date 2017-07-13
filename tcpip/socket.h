#ifndef _SOCKET_H_
#define _SOCKET_H_

struct socket;

typedef void (*udp_recv_cb)(struct socket * sock,struct sk_buff * skb);

struct udp_sock
{
    udp_recv_cb     recv_cb;
};

struct tcp_sock
{
    int mark;
};

struct socket
{
    struct list_head    list;
/* if user bind an ip to the socket,
 * maybe I should bind the network interface 
 * at transport layer????????
 * */
    struct net_device   *nif;           
    uint16_t            local_port;     /*big endian*/
    uint16_t            remote_port;    /*big endian*/
    struct in_addr      local_ip;       /*big endian*/
    struct in_addr      remote_ip;      /*big endian*/

    net_sock_type       sock_type;
    void                *priv;      /* point to udp_sock & tcp_sock*/
};

int init_socket_allocator();
struct socket * open_socket(net_sock_type sock_type);
void close_socket(struct socket * sock);
int bind_socket(struct socket * sock,struct sockaddr_in * addr);

#endif //_SOCKET_H_
