#include "kernel_header.h"
#include "tcpip.h"

struct
{
    net_sock_type       sock_type;       
    struct list_head    head;
    struct mutex        lock;
    uint16_t            sock_count;
} udp_sock_manager;

void init_udp()
{
    mutex_init(&udp_sock_manager.lock,RECU_NORMAL);
    INIT_LIST_HEAD(&udp_sock_manager.head);

    udp_sock_manager.sock_type = SOCK_DGRAM;
    udp_sock_manager.sock_count= 0;
}

int add_udp_socket(struct socket * sock)
{
    if(sock->sock_type != SOCK_DGRAM) {
        return NET_PARAMS_ERR;
    }

    mutex_lock(&udp_sock_manager.lock);
    udp_sock_manager.sock_count++;
    list_add_tail(&sock->list,&udp_sock_manager.head);
    mutex_unlock(&udp_sock_manager.lock);

    return 0;
}

int remove_udp_socket(struct socket * sock)
{
    if(sock->sock_type != SOCK_DGRAM) {
        return NET_PARAMS_ERR;
    }

    mutex_lock(&udp_sock_manager.lock);
    list_del(&sock->list);
    udp_sock_manager.sock_count--;
    mutex_unlock(&udp_sock_manager.lock);

    return 0;
}


static struct socket * search_match_socket(struct net_udp_hdr * udp_hdr,struct net_ip_hdr * ip_hdr)
{
    struct list_head *iter;
    struct socket * sock;

    list_for_each(iter,&udp_sock_manager.head) {

        sock = container_of(iter,struct socket, list);
        if((sock->local_port == udp_hdr->dst_port) && 
                (sock->local_ip.s_addr == INADDR_ANY || sock->local_ip.s_addr == ip_hdr->dst_ip.s_addr)) {
            return sock;
        }
    }

    return NULL;

}

int process_udp_recv(struct sk_buff *skb, struct net_ip_hdr * ip_hdr)
{
    struct net_udp_hdr * udp_hdr = (struct net_udp_hdr *)skb_data(skb);
    struct socket * sock;
    
    mutex_lock(&udp_sock_manager.lock);

    sock = search_match_socket(udp_hdr,ip_hdr);
    if(!sock) {
        free_skb(skb);
        mutex_unlock(&udp_sock_manager.lock);
        /*FIXME: should send icmp packet back*/
        return 0;
    }

    mutex_unlock(&udp_sock_manager.lock);
    
    sock->remote_ip = ip_hdr->src_ip;
    sock->remote_port = udp_hdr->src_port;

    struct udp_sock *udp = sock->priv;

    skb_pull(skb,SIZEOF_UDP_HDR);
    if(udp->recv_cb) {
        (udp->recv_cb)(sock,skb);
    }

    free_skb(skb);

    return 0;
}

int process_udp_send(struct socket * sock,struct sk_buff * skb,struct in_addr * dst_ip,uint16_t dst_port)
{
    if(sock->sock_type != SOCK_DGRAM) {
        free_skb(skb);
        return NET_PARAMS_ERR;
    }

    skb->sock = sock;
    
    sock->remote_ip = *dst_ip;
    sock->remote_port = dst_port;

    skb_push(skb,SIZEOF_UDP_HDR);

    struct net_udp_hdr * udp_hdr = (struct net_udp_hdr *)skb_data(skb);
    
    udp_hdr->src_port = sock->local_port;
    udp_hdr->dst_port = dst_port;
    udp_hdr->len      = htons(skb_data_len(skb));
    udp_hdr->chksum   = 0; /* not using checksum*/

    if(sock->nif) {
        skb_net_if(skb) = sock->nif;
    }

    return process_ip_send(skb,dst_ip,IPPROTO_UDP,DEFAULT_IPV4_TOS,DEFAULT_IPV4_TTL);
}

