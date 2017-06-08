#include "kernel_header.h"
#include "list.h"

extern tcb * cur_tcb_ptr;

LIST_HEAD(wait_list_head);

void add_tcb_to_wait_list(tcb * tcb)
{
    list_add_tail(&tcb->wait_list,&wait_list_head);
}

void remove_tcb_from_wait_list(tcb * tcb)
{
    list_del(&tcb->wait_list);
}

void delay_ms(uint32_t ms)
{
    uint32_t ticks = MS_TO_TICKS(ms);

    if(ticks == 0) { 
        return;
    }

    disable_interrupt();

    cur_tcb_ptr->wait_ticks     = ticks;
    cur_tcb_ptr->wait_object    = NULL;
    cur_tcb_ptr->state          = TASK_PENDING;

    remove_from_ready_rb_tree(cur_tcb_ptr);

    /* add it to waiting list */
    add_tcb_to_wait_list(cur_tcb_ptr);

    enable_interrupt();

    schedule();

    /* comes here means timeout */

    return;
}

/* this function will be called in sys_tick interrupt every tick */
void delay_ticks_procedure(void)
{
    struct list_head * temp,*iter;
    
    int need_schedule = 0; 
   
    disable_interrupt();

    list_for_each_safe(iter,temp,&wait_list_head) {
        
        tcb * cur = list_entry(iter,tcb,wait_list);

        if(cur->wait_ticks >= 1) {
            cur->wait_ticks--;
        }

        if(cur->wait_ticks == 0) {
            
            list_del(&cur->wait_list);

            cur->state    = TASK_RUNNING;     
            cur->pend_ret = PEND_RET_TIMEOUT;

            if(cur->wait_object) {
                remove_tcb_from_rb_tree(cur->wait_object,cur);
            }

            cur->wait_object = NULL;

            add_to_ready_rb_tree(cur);

            need_schedule = 1;
        }
    }

    enable_interrupt();

    if(need_schedule)
        schedule();
}

