#include "kernel_header.h"
#include "bottom_half.h"

#define BH_QUEUE_ITEM           10
#define BH_THREAD_STACK_SIZE    256
struct message_queue bh_queue;
static void * queue_array[BH_QUEUE_ITEM];
static tcb bh_thread;
static stack_element bh_stack[BH_THREAD_STACK_SIZE];


int init_bottom_half(void)
{
    int ret;

    message_queue_init(&bh_queue,(void *)queue_array,sizeof(void *),BH_QUEUE_ITEM);

    /* create the bh thread with high priority */
    ret = create_thread(&bh_thread,bh_stack,BH_THREAD_STACK_SIZE,&thread_loop,0,(void *)&bh_queue,"bh_thread");
    if(ret) {
        return -1;
    }

    return 0;
}

