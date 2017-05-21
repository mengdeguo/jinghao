#include <stdint.h>
#include "sys_tick.h"
#include "bottom_half.h"
#include "timer.h"
#include "platform.h"


static uint32_t sys_tick = 0;

extern void delay_ticks_procedure(void );

uint32_t get_sys_tick()
{
    return sys_tick;
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
