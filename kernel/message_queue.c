#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kernel_header.h"

/* sizeof buffer pointed by array must be item_size * capacity*/
int message_queue_init(struct message_queue * queue,void * array,int item_size, int capacity)
{

    if(array == NULL) {
        return ERR_FAIL;
    }

    queue->queue_head = 0;
    queue->queue_tail = 0;
    queue->queue_size = 0;

    queue->queue_array = (uint8_t *)array;
    queue->capacity = capacity;
    queue->item_size = item_size;

    memset(queue->queue_array,0,item_size * capacity);

    mutex_init(&queue->queue_mutex,RECU_NORMAL);
    condition_init(&queue->queue_condition);

    return 0;
}

void message_queue_destroy(struct message_queue * queue)
{
    queue->queue_head = 0;
    queue->queue_tail = 0;
    queue->queue_size = 0;

    memset(queue->queue_array,0, queue->item_size * queue->capacity);
    mutex_release(&queue->queue_mutex);
    condition_release(&queue->queue_condition);
}

int message_queue_append(struct message_queue * queue,void* item)
{
    int ret = 0;
    mutex_lock(&queue->queue_mutex);

    if(queue->queue_size < queue->capacity) {
        memcpy(&queue->queue_array[queue->queue_head * queue->item_size],item,queue->item_size);

        queue->queue_head = (queue->queue_head + 1) % queue->capacity;
        queue->queue_size ++;
    } else {
        ret = -1;
    }

    condition_signal(&queue->queue_condition);
    mutex_unlock(&queue->queue_mutex);

    return ret;
}

int message_queue_get(struct message_queue * queue,void* p_item, uint32_t wait_ms)
{
    int timeout = 0;

    mutex_lock(&queue->queue_mutex);

    while(!timeout && (queue->queue_size == 0)){
        timeout = condition_wait(&queue->queue_condition,&queue->queue_mutex,wait_ms);
    } 

    if(timeout) {

        mutex_unlock(&queue->queue_mutex);
        return timeout;
    }

    memcpy(p_item,&queue->queue_array[queue->queue_tail * queue->item_size],queue->item_size);
    queue->queue_tail = (queue->queue_tail + 1) % queue->capacity;
    queue->queue_size --;
    
    mutex_unlock(&queue->queue_mutex);

    return 0;
}


