#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include <stdint.h>
#include "rbtree_augmented.h"


typedef enum 
{
    SEMA_READY  = 200,
    SEMA_DESTROY,
}sema_state;

struct semaphore
{
    sema_state          state;
    struct rb_root      root;
    int                 res_count;          // res_count > 0 means source avaliable
    uint32_t            wait_ms; 
};

void semaphore_init(struct semaphore * semaphore, int init_count);
int semaphore_release(struct semaphore * semaphore);
int down_semaphore(struct semaphore * semaphore, uint32_t wait_ms);
int up_semaphore(struct semaphore * semaphore);

int semaphore_get_resource_count(struct semaphore * semaphore);
int is_semaphore_inited(struct semaphore * semaphore);

#endif //_SEMAPHORE_H_
