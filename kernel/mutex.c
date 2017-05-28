#include "mutex.h"
#include "scheduler.h"

extern tcb * cur_tcb_ptr;

/**********************************************************/

void mutex_init(struct mutex *mutex, mutex_recusive recusive)
{
    disable_interrupt();

    mutex->root.rb_node = NULL;
    mutex->avaliable    = AVALIABLE;
    mutex->state        = MUTEX_READY;

    mutex->recusive     = recusive;
    mutex->owner        = NULL;
    mutex->owner_count  = 0;

    enable_interrupt();
}

int mutex_release(struct mutex * mutex)
{
    /*
     * if there's some thread pending on it,
     * it should not be destroied
     * */
    disable_interrupt();
    
    if(mutex->root.rb_node != NULL) {
        enable_interrupt();
        return ERR_FAIL;
    }

    mutex->state = MUTEX_DESTROY;

    enable_interrupt();
    
    return 0;
}

int mutex_lock(struct mutex *mutex)
{
    int ret;

    disable_interrupt();

    if(mutex->state != MUTEX_READY) {
        enable_interrupt();
        return ERR_FAIL;
    }

    if(mutex->avaliable == AVALIABLE) {
        mutex->avaliable = ACQUIRED;

        if(mutex->recusive == RECUSIVE) {
            mutex->owner = cur_tcb_ptr;
            mutex->owner_count++;
        }

        /* here the value pend_ret could be ignored */
        cur_tcb_ptr->pend_ret = PEND_RET_OK;

        enable_interrupt();

        return 0;
    } 

    if((mutex->recusive == RECUSIVE) && (mutex->owner == cur_tcb_ptr)) {
        mutex->owner_count++;
        
        /* here the value pend_ret could be ignored */
        cur_tcb_ptr->pend_ret = PEND_RET_OK;

        enable_interrupt();

        return 0;
    }

    /* 
     * the mutex is acquired by some other thread,
     * so should put current thread to waiting list 
     * */
    cur_tcb_ptr->state          = TASK_PENDING;
    cur_tcb_ptr->wait_object    = &mutex->root;

    remove_from_ready_rb_tree(cur_tcb_ptr);

    add_tcb_to_rb_tree(&mutex->root,cur_tcb_ptr);

    enable_interrupt();

    schedule();

    /* comes here, means acquire mutex, it can not be timeout */

    /* do some check about the pend_ret */
    ret = (cur_tcb_ptr->pend_ret == PEND_RET_OK) ? 0 : ERR_TIMEOUT;

    return ret;
}

int mutex_unlock(struct mutex * mutex)
{
    disable_interrupt();

    if(mutex->state != MUTEX_READY) {
        enable_interrupt();

        return ERR_FAIL;
    }

    if(mutex->recusive == RECUSIVE) {
        mutex->owner_count --;

        if(mutex->owner_count > 0) {
            enable_interrupt();
            return 0;
        }
    }

    mutex->avaliable = AVALIABLE;

    /* check is there some pending thread */
    if(!mutex->root.rb_node) {
        enable_interrupt();

        return 0;
    }

    /* get highest priority  */

    tcb * active_tcb = get_highest_priority_tcb(&mutex->root);
    if(!active_tcb) {
        enable_interrupt();

        return ERR_FAIL;
    }

    remove_tcb_from_rb_tree(&mutex->root,active_tcb);

    active_tcb->state = TASK_RUNNING;

    /* here the value pend_ret could be ignored */
    active_tcb->pend_ret       = PEND_RET_OK;
    active_tcb->wait_object    = NULL;
    
    add_to_ready_rb_tree(active_tcb);
    
    mutex->avaliable = ACQUIRED;

    if(mutex->recusive == RECUSIVE) {
        mutex->owner = active_tcb;
        mutex->owner_count++;
    }

    enable_interrupt();

    schedule();

    return 0;
}

