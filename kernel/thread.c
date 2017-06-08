#include <stdio.h>
#include <stdint.h>
#include "kernel_header.h"
#include "platform.h"

tcb * cur_tcb_ptr = NULL;
tcb * highest_priority_tcb_ptr = NULL;

static tcb idle;
static stack_element idle_stack[IDLE_STACK_SIZE];


#define IDLE_HOOK_COUNT 5
static void (*g_idle_hooks[IDLE_HOOK_COUNT])(void) = { NULL };

int install_idle_hook(void (*hook)(void))
{
    int i = 0;
    
    while(i < IDLE_HOOK_COUNT) {
        if(g_idle_hooks[i] == NULL) {
            g_idle_hooks[i] = hook;
            break;
        }

        ++i;
    }
    
    return (i < IDLE_HOOK_COUNT) ? 0 : -1;
}

void remove_idle_hook(void (*hook)(void))
{
    int i = 0;

    while(i < IDLE_HOOK_COUNT) {
        if(g_idle_hooks[i] == hook) {
            g_idle_hooks[i] = NULL;
            break;
        } 

        ++i;
    }
}

static void execute_idle_hook()
{
    int i = 0;

    while(i < IDLE_HOOK_COUNT) {
        if(g_idle_hooks[i]) {
            g_idle_hooks[i]();
        }

        i++;
    }
}

extern void switch_to_thread_mode();


void idle_thread(void * args)
{
    (void)args;

    while(1) {
        execute_idle_hook();

        __asm volatile ("wfi");
    }
}

void find_high_ready_thread(void )
{
    highest_priority_tcb_ptr = get_highest_ready_tcb();
    if(!highest_priority_tcb_ptr) {
        OS_LOG("can not find highest_priority thread \r\n");
        while(1) {
        }
        return;
    }
}

/* must be called after create one thread */
void start_os()
{
    int ret;

    /* create idle thread */
    ret = create_thread(&idle,idle_stack,IDLE_STACK_SIZE,idle_thread,
            IDLE_THREAD_PRIORITY,(void *)0,"idle_thread");
    if(ret) {
        return;
    }

    if(init_bottom_half()) {

        //create bottom_half thread error
        return;
    }

    init_timer_manager();

    /* the first high_rdy task must not be idle */
    find_high_ready_thread();

    mark_start_tick();

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

    if(highest_priority_tcb_ptr == &idle) {
        mark_enter_idle();
    }

    if(cur_tcb_ptr == &idle) {
        mark_exit_idle();
    }

#if ENABLE(RTOS_DEBUG)
    schedule_to_thread(highest_priority_tcb_ptr);
#endif
    
    /* trigger pendsv exception */
    NVIC_INT_CTRL = NVIC_PENDSVSET;

    //sched_unlock();
    enable_interrupt();
}

stack_element *stack_init(thread_entry p_entry, void *p_arg, stack_element *p_stk_base, uint32_t stk_size)
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


int create_thread(tcb * m_tcb, stack_element* stack,uint32_t stack_size, thread_entry entry,task_priority priority,void * arg,const char * name)
{
    if(!m_tcb) {
        return ERR_FAIL;
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

    return 0;
}

void destroy_thread(tcb * thread)
{
    if(!thread)
        return;

    thread->state = TASK_NONE;
    
}

const char * get_current_thread_name()
{
    return cur_tcb_ptr ? cur_tcb_ptr->name : NULL;
}


