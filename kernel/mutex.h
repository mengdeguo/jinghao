#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <stdint.h>
#include "thread.h"
#include "rb_int_operation.h"

typedef enum 
{
    MUTEX_READY = 100,
    MUTEX_DESTROY,
}mutex_state;

typedef enum 
{
    ACQUIRED = 110,
    AVALIABLE,
}mutex_avaliable;


typedef enum {
    RECU_NORMAL = 120,
    RECUSIVE,
} mutex_recusive;

struct mutex
{
    mutex_state         state;
    struct rb_root      root;       // the pending thread will wait on this rb tree 
    mutex_avaliable     avaliable;

    mutex_recusive      recusive;
    tcb                 *owner;
    int                 owner_count;
};

int mutex_lock(struct mutex *mutex);
int mutex_unlock(struct mutex *mutex);
void mutex_init(struct mutex *mutex, mutex_recusive recusive);
int mutex_release(struct mutex * mutex);

#endif //_MUTEX_H_
