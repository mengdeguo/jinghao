#include "condition.h"

extern tcb * cur_tcb_ptr;

void condition_init(struct condition * cond)
{
    disable_interrupt();

    cond->state = COND_READY;
    
    enable_interrupt();

    semaphore_init(&cond->sema,0);
}

int condition_release(struct condition * cond)
{
    if(semaphore_release(&cond->sema) != 0) {
        return -1;
    }

    disable_interrupt();

    cond->state = COND_DESTROY;

    enable_interrupt();

    return 0;
}

int condition_wait(struct condition * cond, struct mutex * mutex,uint32_t wait_ms)
{
    /* the mutex has already locked outsize the function, so it's threadsafe */
    if(cond->state != COND_READY){
        return -1;
    }

    int ret;

    ret = mutex_unlock(mutex);
    if(ret) {
        return ret;
    }

    ret = down_semaphore(&cond->sema,wait_ms);
    
    /* lock again, so the environment is same as that before entering the function */
    mutex_lock(mutex);

    return ret;
}

int condition_signal(struct condition * cond)
{
    /* the mutex has already locked outsize the function, so it's threadsafe */
    if(cond->state != COND_READY) {
        return -1;
    }

    return up_semaphore(&cond->sema);
}

