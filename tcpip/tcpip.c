#include "list.h"
#include "kernel_header.h"
#include "tcpip.h"
#include "platform.h"

static struct net_device netdev_loop;

static struct 
{
    struct list_head    head;
    int                 netdev_count;
    struct mutex        mutex;
    struct net_device   *default_nif;
} netdev_list;

static void init_netdev_list()
{
    INIT_LIST_HEAD(&netdev_list.head);
    mutex_init(&netdev_list.mutex,RECU_NORMAL);
    netdev_list.netdev_count = 0;
    netdev_list.default_nif = NULL;
}

int register_netdev(struct net_device *dev)
{
    if(!dev) {
        return NET_PARAMS_ERR;
    }

    mutex_lock(&netdev_list.mutex);
    list_add(&dev->list,&netdev_list.head);
    netdev_list.netdev_count ++;
    mutex_unlock(&netdev_list.mutex);

    return 0;
}

void set_default_netif(struct net_device * nif)
{
    mutex_lock(&netdev_list.mutex);
    netdev_list.default_nif = nif; 
    mutex_unlock(&netdev_list.mutex);
}

struct net_device * find_nif_by_ip(struct in_addr * ip)
{
    struct net_device * nif = NULL;
    struct list_head * iter;

    mutex_lock(&netdev_list.mutex);

    list_for_each(iter,&netdev_list.head) {
        nif = container_of(iter,struct net_device,list);
        if(ip->s_addr == nif->ip.s_addr) {
            mutex_unlock(&netdev_list.mutex);
            return nif;
        }
    }

    mutex_unlock(&netdev_list.mutex);

    return netdev_list.default_nif;
}

/* simple router algorithm used by ip layer */
struct net_device * search_appropriate_nif(struct in_addr * ip)
{
    struct net_device * nif = NULL;
    struct list_head * iter;

    mutex_lock(&netdev_list.mutex);

    list_for_each(iter,&netdev_list.head) {
        nif = container_of(iter,struct net_device,list);
        if((nif->if_flags & NETIF_FLAG_UP) && 
                ((ip->s_addr & nif->netmask.s_addr) == (nif->ip.s_addr & nif->netmask.s_addr))) {

            mutex_unlock(&netdev_list.mutex);
            return nif;
        }
    }

    mutex_unlock(&netdev_list.mutex);

    return netdev_list.default_nif;
}

/*****************net device operation********************/
void setup_netdev(struct net_device * dev, const char * name)
{
    mutex_init(&dev->mutex,RECU_NORMAL);
    INIT_LIST_HEAD(&dev->list);
    dev->mtu = DEFAULT_ETH_MTU;
    dev->if_flags = NETIF_FLAG_DOWN;
    dev->drv_private = NULL;
}

void set_netdev_drv_private(struct net_device *dev,void * data)
{
    mutex_lock(&dev->mutex);
    dev->drv_private = data;
    mutex_unlock(&dev->mutex);
}

void * get_netdev_drv_private(struct net_device *dev)
{
    return dev->drv_private;
}

void set_netdev_status_notifier(struct net_device *dev, netdev_status_notifier notifier)
{
    mutex_lock(&dev->mutex);
    dev->ops.status_notifier = notifier;
    mutex_unlock(&dev->mutex);
}

void set_netdev_output_op(struct net_device *dev,netdev_output output)
{
    mutex_lock(&dev->mutex);
    dev->ops.output = output;
    mutex_unlock(&dev->mutex);
}

void set_netdev_down(struct net_device * dev)
{
    if(!dev) {
        return;
    }

    mutex_lock(&dev->mutex);

    if (dev->if_flags & NETIF_FLAG_UP) {
        dev->if_flags &= NETIF_FLAG_DOWN;

        /* clear the arp cache for the net device*/
        netdev_arp_cache_clean(dev);

        if(dev->ops.status_notifier){
            dev->ops.status_notifier(dev);
        }
    }

    mutex_unlock(&dev->mutex);
}

void set_netdev_up(struct net_device * dev)
{
    if(!dev) {
        return;
    }

    mutex_lock(&dev->mutex);

    if (!(dev->if_flags & NETIF_FLAG_UP)) {
        dev->if_flags |= NETIF_FLAG_UP;

        if(dev->ops.status_notifier){
            dev->ops.status_notifier(dev);
        }

        if (dev->if_flags & NETIF_FLAG_LINK_UP) {

            /* when the interface up we would like to send a "gratuitous ARP" */
            send_gratuitous_arp_request(dev);
        }
    }

    mutex_unlock(&dev->mutex);
}
/***********************************************************/

static void init_netdev_loop()
{
    setup_netdev(&netdev_loop,"lo");

    netdev_loop.ip.s_addr = IP4_ADDR_LOOPBACK;
    netdev_loop.netmask.s_addr = IP4_NETMASK_LOOPBACK;
    netdev_loop.gw.s_addr = IP4_ADDR_LOOPBACK;

    netdev_loop.if_flags = NETIF_FLAG_UP | NETIF_FLAG_LINK_UP;
    set_netdev_status_notifier(&netdev_loop,NULL);

    /*FIXME: add output method, which may just call process_eth_recv */
    register_netdev(&netdev_loop);
}

#define TCPIP_QUEUE_ITEM                20
#define TCPIP_THREAD_STACK_SIZE         1024 
struct message_queue tcpip_queue;
static void * queue_array[TCPIP_QUEUE_ITEM];

static tcb tcpip_thread;
static stack_element tcpip_stack[TCPIP_THREAD_STACK_SIZE];

int create_tcpip_thread()
{
    int ret;
    
    message_queue_init(&tcpip_queue,(void *)queue_array,sizeof(void *),TCPIP_QUEUE_ITEM);

    ret = create_thread(&tcpip_thread,tcpip_stack,TCPIP_THREAD_STACK_SIZE,&thread_loop,TCPIP_THREAD_PRIO,&tcpip_queue,"tcpip");

    return ret;
}


int tcpip_init()
{
    if(create_tcpip_thread()) {
        OS_LOG("create tcpip thread fail \r\n");
        return NET_INIT_ERR;
    }

    init_netdev_list();
    init_netdev_loop();
    init_skb_allocator();
    init_socket_allocator();

    init_arp();
    init_udp();

    return 0;
}
