#include <stdint.h>
#include "kernel_header.h"
#include "platform.h"


static uint32_t sys_tick = 0;

extern void delay_ticks_procedure(void );

uint32_t get_sys_tick()
{
    return sys_tick;
}

void update_systick(uint32_t ticks)
{
    sys_tick += ticks;
}

int get_time_gap(uint32_t t) 
{
    return (int)(sys_tick - t);
}

void SysTick_IRQHandler()
{
    if(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) {
        sys_tick += 1;

        schedule_bh_thread(delay_ticks_procedure,0);
        schedule_bh_thread(timer_ticks_procedure,0);
    }
}

void sys_tick_init(void)
{
    SysTick->LOAD  = CORE_CLK/SYSTICK_HZ - 1;               /* set reload register */
    SysTick->VAL   = 0;                                     /* Load the SysTick Counter Value */
    NVIC_SetPriority (SysTick_IRQn, SYSTICK_IRQ_PRI);       /* A little higher than pendsv Priority */ 
    
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                     SysTick_CTRL_TICKINT_Msk   |
                     SysTick_CTRL_ENABLE_Msk;   /* Enable SysTick IRQ and SysTick Timer */
}


#if ENABLE(CALCULATE_CPU_LOAD)
static uint32_t system_start_tick   = 0;
static uint32_t total_idle_ticks    = 0;

static uint32_t enter_idle_tick     = 0;

void mark_start_tick()
{
    system_start_tick = get_sys_tick();
}

void mark_enter_idle()
{
    enter_idle_tick = get_sys_tick();
}

void mark_exit_idle()
{
    /* only used when test, so don't worry about data overflow */
    total_idle_ticks += (get_sys_tick() - enter_idle_tick );
}

int get_idle_percentage()
{
    return total_idle_ticks * 100 / (get_sys_tick() - system_start_tick);
}

#else //ENABLE(CALCULATE_CPU_LOAD)

void mark_start_tick()
{

}

void mark_enter_idle()
{

}

void mark_exit_idle()
{

}

int get_idle_percentage()
{
    return 0;
}

#endif //ENABLE(CALCULATE_CPU_LOAD)
