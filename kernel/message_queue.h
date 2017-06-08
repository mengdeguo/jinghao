#ifndef _MESSAGE_QUEUE_H_ 
#define _MESSAGE_QUEUE_H_

/**************************************************
* capacity == max number of items can be stored in the queue 
***************************************************/
struct message_queue
{
    uint8_t *           queue_array;
    int                 queue_head;
    int                 queue_tail;
    int                 queue_size;
    int                 item_size;
    int                 capacity;
    struct mutex        queue_mutex;
    struct condition    queue_condition;
};

int message_queue_init(struct message_queue * queue,void * array,int item_size, int capacity);
void message_queue_destroy(struct message_queue * queue);
int message_queue_append(struct message_queue * queue,void* item);
int message_queue_get(struct message_queue * queue,void* p_item, uint32_t wait_ms);

#endif //_MESSAGE_QUEUE_H_
