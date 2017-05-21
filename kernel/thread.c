#include <stdio.h>
#include <stdint.h>
#include "thread.h"
#include "rbtree_augmented.h"
#include "scheduler.h"
#include "bottom_half.h"
#include "timer.h"
#include "platform.h"

tcb * cur_tcb_ptr = NULL;
tcb * highest_priority_tcb_ptr = NULL;

extern void switch_to_thread_mode();

void idle_thread(void * args)
{
    (void)args;

    while(1) {
        __asm volatile ("wfi");
    }
}

void find_high_ready_thread(void )
{
    highest_priority_tcb_ptr = get_highest_ready_tcb();
    if(!highest_priority_tcb_ptr) {
        return;
    }
}

/* must be called after create one thread */
void start_os()
{
    /* create idle thread */
    tcb * idle = create_thread(IDLE_STACK_SIZE,idle_thread,
            IDLE_THREAD_PRIORITY,(void *)0,"idle_thread");
    if(!idle) {
        return;
    }

    if(init_bottom_half()) {

        //create bottom_half thread error
        return;
    }

    init_timer_manager();

    find_high_ready_thread();

    switch_to_thread_mode();
}

void set_cur_tcb(tcb * cur)
{
    cur_tcb_ptr = cur;
}

void set_high_ready_tcb(tcb * high_rdy)
{
    highest_priority_tcb_ptr = high_rdy;
}

void thread_safe_return()
{
    while(1) {
        __asm volatile("wfi");
    }
}

void schedule(void) 
{
    //sched_lock();
    disable_interrupt();

    find_high_ready_thread();

    /* check if current thread is the highest priority thread */ 
    if(highest_priority_tcb_ptr == cur_tcb_ptr) { 

        //sched_unlock();
        enable_interrupt();
        return;
    }
    
    /* trigger pendsv exception */
    NVIC_INT_CTRL = NVIC_PENDSVSET;

    //sched_unlock();
    enable_interrupt();
}

stack_element *stack_init(thread_entry       p_entry, void          *p_arg, stack_element       *p_stk_base, uint32_t       stk_size)
{
    stack_element  *p_stk;

    p_stk = &p_stk_base[stk_size];                /* Load stack pointer                                     */
                                                  /* Registers stacked as if auto-saved on exception        */
    *--p_stk = (stack_element)0x01000000u;              /* xPSR, Thumb state                                                  */
    *--p_stk = (stack_element)p_entry;                  /* Entry Point                                            */
    *--p_stk = (stack_element)thread_safe_return;       /* R14 (LR)                                               */
    *--p_stk = (stack_element)0;                        /* R12                                                    */
    *--p_stk = (stack_element)0;                        /* R3                                                     */
    *--p_stk = (stack_element)0;                        /* R2                                                     */
    *--p_stk = (stack_element)0;                        /* R1                                                     */
    *--p_stk = (stack_element)p_arg;                    /* R0 : argument                                          */
                                                  /* Remaining registers saved on process stack             */
    *--p_stk = (stack_element)0;                        /* R11                                                    */
    *--p_stk = (stack_element)0;                        /* R10                                                    */
    *--p_stk = (stack_element)0;                        /* R9                                                     */
    *--p_stk = (stack_element)0;                        /* R8                                                     */
    *--p_stk = (stack_element)0;                        /* R7                                                     */
    *--p_stk = (stack_element)0;                        /* R6                                                     */
    *--p_stk = (stack_element)0;                        /* R5                                                     */
    *--p_stk = (stack_element)0;                        /* R4                                                     */

    return (p_stk);
}

tcb * create_thread(uint32_t stack_size, thread_entry entry,task_priority priority,void * arg,const char * name)
{
    tcb * m_tcb = (tcb *)allocate(sizeof(tcb));
    if(!m_tcb) {
        return NULL;
    }

    stack_element * stack = (stack_element *) allocate(stack_size * sizeof(stack_element));
    if(!stack) {
        goto error_handle;
    }

    m_tcb->stack_array  = stack;
    m_tcb->stack_size   = stack_size;
    m_tcb->priority     = priority;
    m_tcb->name         = name;
    m_tcb->state        = TASK_RUNNING;
    m_tcb->pend_ret     = PEND_RET_OK;
    m_tcb->wait_object  = NULL;
    m_tcb->wait_ticks   = 0;

    INIT_LIST_HEAD(&m_tcb->wait_list);

    m_tcb->tsp = stack_init(entry,arg,stack,stack_size);

    /* add task to ready list */
    disable_interrupt();
    add_to_ready_rb_tree(m_tcb);
    enable_interrupt();

    return m_tcb;

error_handle:
    if(m_tcb) {
        deallocate(m_tcb);
    }

    return NULL;
}

void destroy_thread(tcb * thread)
{
    if(!thread)
        return;

    if(thread->stack_array)
        deallocate(thread->stack_array);

    deallocate(thread);
}




