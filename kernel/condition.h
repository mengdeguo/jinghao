#ifndef _CONDITION_H_
#define _CONDITION_H_

#include <stdint.h> 
#include "semaphore.h"
#include "mutex.h"

/* works like pthread_condition_t*/

typedef enum 
{
    COND_READY  =   300,
    COND_DESTROY,
}cond_state;

struct condition
{
    cond_state          state;
    struct semaphore    sema;
};


void condition_init(struct condition * cond);
int condition_release(struct condition * cond);
int condition_wait(struct condition * cond, struct mutex * mutex,uint32_t wait_ms);
int condition_signal(struct condition * cond);

#endif //_CONDITION_H_
