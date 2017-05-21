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

struct mutex
{
    mutex_state         state;
    struct rb_root      root;       // the pending thread will wait on this rb tree 
    mutex_avaliable     avaliable;
};

int mutex_lock(struct mutex *mutex);
int mutex_unlock(struct mutex *mutex);
void mutex_init(struct mutex *mutex);
int mutex_release(struct mutex * mutex);

#endif //_MUTEX_H_
