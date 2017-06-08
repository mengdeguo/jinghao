#ifndef _KERNEL_HEADER_H_
#define _KERNEL_HEADER_H_

#include <stdint.h>
#include "kernel_config.h"

/*
 * according to the size of the stack elements
 * */
typedef uint32_t    stack_element;

/*
 * task priority range :0-255 ,255 is used for idle task
 * */
typedef uint8_t     task_priority;

enum error_code
{
    ERR_FAIL    = -1,
    ERR_TIMEOUT = -1000,
};


typedef enum  {
    TASK_NONE = 0,
    TASK_RUNNING,
    TASK_PENDING,
    TASK_STOPED,
}task_state;


typedef enum {
    PEND_RET_OK         = 0,
    PEND_RET_TIMEOUT,
    PEND_RET_FAIL,
}pend_result;

#define KERNEL_WAIT_FOREVER  0xFFFFFFFF
#define KERNEL_NO_WAIT   0

#define MS_TO_TICKS(ms)     ((ms) * SYSTICK_HZ / 1000 )
#define TICKS_TO_MS(ticks)  ((ticks) * 1000 / SYSTICK_HZ)

#include "thread.h"
#include "scheduler.h"
#include "sys_tick.h"
#include "mutex.h"
#include "time.h"
#include "timer.h"
#include "semaphore.h"
#include "condition.h"
#include "message_queue.h"
#include "bottom_half.h"
#include "rw_lock.h"
#include "os_debug.h"

#endif //_KERNEL_HEADER_H_

