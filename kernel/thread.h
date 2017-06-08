#ifndef _THREAD_H_
#define _THREAD_H_

#include <stdint.h>
#include "rbtree_augmented.h"
#include "list.h"
#include "platform.h"


#define IDLE_THREAD_PRIORITY    (0xff)
#define IDLE_STACK_SIZE         (128)

#define  NVIC_INT_CTRL                      *((uint32_t *)0xE000ED04)
#define  NVIC_PENDSVSET                                    0x10000000

//define in switch.s
extern uint32_t NVIC_PENDSV_PRI;

/*
 * lock schedule
 * many methods to lock the scheduler, the most common way is disable Interrupt, then increase lock_count number
 * here only disable pendsv Execution 
 * */
#define sched_lock() __set_BASEPRI(NVIC_PENDSV_PRI)
#define sched_unlock() __set_BASEPRI(0)

//disalbe irq, defined in core_cmFunc.h
#define disable_interrupt() __disable_irq()
#define enable_interrupt() __enable_irq()

typedef void (* thread_entry)(void *);

typedef struct tcb_t
{
    stack_element       *tsp;               //thread stack pointer, must be the first element
    stack_element       *stack_array;       //store the ptr to the dynamic allocated stack 
    size_t              stack_size;         //store the size of the stack
    task_priority       priority;           //priority of the thread
    task_state          state; 
    pend_result         pend_ret;           //the reason for returning from pending status
    struct rb_node      node;
    struct list_head    wait_list;          //wait for timeout
    struct rb_root*     wait_object;        //the object which the thread is pending on
    uint32_t            wait_ticks;
    const char *        name;               //name of the thread
}tcb;

int create_thread(tcb * m_tcb,stack_element * stack,uint32_t stack_size, thread_entry entry,task_priority priority,void * arg,const char * name);
void destroy_thread(tcb * thread);
void set_cur_tcb(tcb * cur);
void set_high_ready_tcb(tcb * high_rdy);
void start_os(void);
void schedule(void);
const char * get_current_thread_name();

int install_idle_hook(void (*hook)(void));
void remove_idle_hook(void (*hook)(void));

#endif //_THREAD_H_
