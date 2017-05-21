#ifndef _BOTTOM_HALF_H_
#define _BOTTOM_HALF_H_

#include "message_queue.h"
#include "cross_thread_copier.h"

extern struct message_queue bh_queue;

#define schedule_bh_thread(func,para_count, ...)                                        \
        do {                                                                            \
            call_task_on_thread_queue(&bh_queue,(general_cross_thread_task)func,        \
                    NULL,para_count, ##__VA_ARGS__);                                    \
        }while(0)


int init_bottom_half(void);

#endif //_BOTTOM_HALF_H_
