#ifndef _MESSAGE_QUEUE_H_ 
#define _MESSAGE_QUEUE_H_

#include "mutex.h"
#include "condition.h"

#define MAX_QUEUE_ITEM  (10)

struct message_queue
{
    void *              queue_array[MAX_QUEUE_ITEM];
    int                 queue_head;
    int                 queue_tail;
    int                 queue_size;
    struct mutex        queue_mutex;
    struct condition    queue_condition;
};

int message_queue_init(struct message_queue * queue);
void message_queue_destroy(struct message_queue * queue);
int message_queue_append(struct message_queue * queue,void* item);
int message_queue_get(struct message_queue * queue,void** p_item, uint32_t wait_ms);

#endif //_MESSAGE_QUEUE_H_
