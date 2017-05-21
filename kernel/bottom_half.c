#include "bottom_half.h"

struct message_queue bh_queue;
static tcb * bh_thread = NULL;

int init_bottom_half(void)
{
    /* create the bh thread with high priority */
    bh_thread = create_thread(256,&thread_loop,0,(void *)&bh_queue,"bh_thread");
    
    if(!bh_thread) {
        return -1;
    }

    return 0;
}

