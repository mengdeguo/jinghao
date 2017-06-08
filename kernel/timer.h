#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdint.h>
#include "list.h"

typedef enum
{
    ONE_SHOT = 400,
    REPEAT,
}timer_type;

typedef enum {
    TIMER_ACTIVE = 410,
    TIMER_DEACTIVE,
}timer_status;

typedef void (*timer_handle)(void * param);

struct timer
{
    struct list_head    list;
    void *              param;
    timer_handle        handle;
    uint32_t            period_ticks;
    uint32_t            delay_ticks;
    timer_type          type;
    timer_status        status;
};

void init_timer_manager(void);
void init_timer(struct timer * timer,timer_handle handle,void * param,uint32_t period_ms,uint32_t delay_ms,timer_type type);
int start_timer(struct timer * timer);
int stop_timer(struct timer * timer);
void timer_ticks_procedure(void);
int is_timer_active(struct timer * timer);
void change_timer(struct timer * timer,uint32_t period_ms,uint32_t delay_ms);

#endif //_TIMER_H_
