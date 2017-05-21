#include "semaphore.h"
#include "scheduler.h"
#include "time.h"

extern tcb * cur_tcb_ptr;

void semaphore_init(struct semaphore * semaphore, int init_count)
{
    disable_interrupt();

    semaphore->state            = SEMA_READY;
    semaphore->root.rb_node     = NULL;
    semaphore->res_count        = init_count;
    semaphore->wait_ms          = 0;

    enable_interrupt();
}

int semaphore_release(struct semaphore * semaphore)
{
    disable_interrupt();

    if(semaphore->root.rb_node) {
        enable_interrupt();

        return -1;
    }

    semaphore->state = SEMA_DESTROY;

    enable_interrupt();

    return 0;
}

/* wait_ms == 0 means wait for ever */
int down_semaphore(struct semaphore * semaphore, uint32_t wait_ms)
{
    int ret;

    disable_interrupt();

    if(semaphore->state != SEMA_READY) {
        enable_interrupt();

        return ERR_FAIL;
    }

    if(semaphore->res_count > 0) {
        semaphore->res_count --;

        cur_tcb_ptr->pend_ret = PEND_RET_OK;

        enable_interrupt();

        return 0;
    }

    /* should pending the current thread */

    cur_tcb_ptr->state          = TASK_PENDING;
    cur_tcb_ptr->wait_object    = &semaphore->root;

    remove_from_ready_rb_tree(cur_tcb_ptr);
    add_tcb_to_rb_tree(&semaphore->root,cur_tcb_ptr);

    if(wait_ms > 0) {
        cur_tcb_ptr->wait_ticks     = MS_TO_TICKS(wait_ms);     
        add_tcb_to_wait_list(cur_tcb_ptr);
    }

    enable_interrupt();

    schedule();

    /* check the return reason */
    /*  using -1 to represent wait timeout & down error */
    ret = (cur_tcb_ptr->pend_ret == PEND_RET_OK) ? 0 : ERR_TIMEOUT;

    return ret;
}

int up_semaphore(struct semaphore * semaphore)
{
    disable_interrupt();

    if(semaphore->state != SEMA_READY) {
        enable_interrupt();

        return -1;
    }

    semaphore->res_count ++;

    if(!semaphore->root.rb_node) {
        enable_interrupt();

        return 0;
    }

    /* should active the highest priority waiting thread */
    tcb * active_tcb = get_highest_priority_tcb(&semaphore->root);
    if(!active_tcb) {
        enable_interrupt();

        return -1;
    }

    remove_tcb_from_rb_tree(&semaphore->root,active_tcb);

    /* check if active_tcb on wait list */
    if( (active_tcb->wait_list.next != &active_tcb->wait_list) && (active_tcb->wait_list.next != NULL) ) {
        remove_tcb_from_wait_list(active_tcb);
    }

    active_tcb->state       = TASK_RUNNING;
    active_tcb->pend_ret    = PEND_RET_OK;
    active_tcb->wait_object = NULL;
    active_tcb->wait_ticks  = 0;

    add_to_ready_rb_tree(active_tcb);

    semaphore->res_count --;

    enable_interrupt();

    schedule();

    return 0;
}

