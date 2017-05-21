#include "timer.h"
#include "mutex.h"
#include "time.h"

struct timer_manager
{
    struct list_head    head;
    uint32_t            count;
    struct mutex        mutex;
};

static struct timer_manager manager;

void init_timer_manager(void)
{
    manager.count = 0;
    mutex_init(&manager.mutex);
    INIT_LIST_HEAD(&manager.head);
}

void init_timer(struct timer * timer,timer_handle handle,void * param,uint32_t period_ms,uint32_t delay_ms,timer_type type)
{
    INIT_LIST_HEAD(&timer->list);

    timer->param        = param;
    timer->handle       = handle;
    timer->period_ticks = MS_TO_TICKS(period_ms);
    timer->delay_ticks  = MS_TO_TICKS(delay_ms);
    timer->type         = type;
}

static void add_timer_without_lock(struct timer * timer)
{
    manager.count ++;

    struct list_head * iter = NULL;

    list_for_each(iter,&manager.head) {
        
        struct timer * cur = list_entry(iter,struct timer, list);

        if(timer->delay_ticks <= cur->delay_ticks) {
            cur->delay_ticks -= timer->delay_ticks;

            list_add_tail(&timer->list,&cur->list);

            break;
        }

        timer->delay_ticks -= cur->delay_ticks;
    }

    if(iter == &manager.head) {
        list_add_tail(&timer->list,&manager.head);
    }
}

int start_timer(struct timer * timer)
{
    if(!timer->handle || ((timer->type != ONE_SHOT) && (timer->type != REPEAT))) {
        return -1;
    }

    /* compare dealy time, then add timer to correct position */
    mutex_lock(&manager.mutex);

    add_timer_without_lock(timer);

    mutex_unlock(&manager.mutex);

    return 0;
}

int stop_timer(struct timer * timer)
{
    /* check param */
    if(!timer->handle || ((timer->type != ONE_SHOT) && (timer->type != REPEAT))) {
        return -1;
    }

    mutex_lock(&manager.mutex);

    struct list_head * iter = NULL;

    list_for_each(iter,&manager.head) {
        
        struct timer * cur = list_entry(iter,struct timer,list);
        if(cur == timer) {
            list_del(&timer->list);
            break;
        }
    }
    
    if(list_entry(iter,struct timer,list) == timer){
        manager.count --;
    }

    mutex_unlock(&manager.mutex);

    return 0;
}

void timer_ticks_procedure(void)
{
    mutex_lock(&manager.mutex);

    if(manager.count < 1) {
        mutex_unlock(&manager.mutex);

        return;
    }

    struct list_head * iter, * tmp;

    list_for_each_safe(iter,tmp,&manager.head) {

        struct timer * cur = list_entry(iter,struct timer,list);

        if(cur->delay_ticks > 1) {
            cur->delay_ticks--;
            break;
        }

        list_del(&cur->list);

        manager.count--;

        /* fire the timer */
        (cur->handle)(cur);
        
        /* check if the timer is periodic */
        if(cur->type == REPEAT) {
            cur->delay_ticks = cur->period_ticks;

            add_timer_without_lock(cur);
        }
    
    }

    mutex_unlock(&manager.mutex);
}

