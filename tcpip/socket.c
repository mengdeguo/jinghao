#include <string.h>
#include "kernel_header.h"
#include "tcpip.h"

#define SOCKET_COUNT    (10)

NETOBJ_CACHE(struct socket,socket,SOCKET_COUNT);

int init_socket_allocator()
{
    return init_netobj_cache(&socket_cache);
}

struct socket * open_socket(net_sock_type sock_type)
{
    if(sock_type != SOCK_STREAM && sock_type != SOCK_DGRAM) {
        return NULL;
    }

    struct socket * sock = netobj_cache_alloc(&socket_cache);
    if(sock) { 
        void * tmp;
        int size; 
        
        if(sock_type == SOCK_DGRAM) {
            size = sizeof(struct udp_sock);
        } else {
            size = sizeof(struct tcp_sock);
        }

        tmp = allocate(size);
        if(!tmp) {
            netobj_cache_free(&socket_cache,(void *)sock);
            return NULL;
        }

        memset(sock,0,sizeof(struct socket));
        memset(tmp,0,size);

        sock->priv = tmp;
        INIT_LIST_HEAD(&sock->list);
        sock->sock_type = sock_type;

        if(sock_type == SOCK_DGRAM) {
            add_udp_socket(sock);
        } else {
            /*FIXME: will implement tcp in near future*/
            //add_tcp_socket(sock);
        }
    }

    return sock;
}

void close_socket(struct socket * sock) 
{
    if(!sock) {
        return;
    }

    if(sock->sock_type == SOCK_DGRAM) {
        remove_udp_socket(sock);
    } else {
        /*FIXME: will implement tcp in near future*/
        //remove_tcp_socket(sock);
    }

    if(sock->priv) {
        deallocate(sock->priv);
    }

    netobj_cache_free(&socket_cache,(void *)sock);
}

int bind_socket(struct socket * sock,struct sockaddr_in * addr)
{
    if(!sock) {
        return NET_PARAMS_ERR;
    }

    /* currently only support IPV4*/
    if(addr->sin_family != AF_INET) {
        return NET_PARAMS_ERR;
    }

    sock->local_port = addr->sin_port;
    sock->local_ip   = addr->sin_addr;

    /*FIXME: shold I do it here ?*/
    if(sock->local_ip.s_addr != INADDR_ANY) {
        /*find the correct network interface */
        sock->nif = find_nif_by_ip(&sock->local_ip);
    }

    return 0;
}
