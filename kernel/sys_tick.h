#ifndef _SYS_TICK_H_
#define _SYS_TICK_H_

#include <stdint.h>

#define SYSTICK_HZ  (1000)
#define CORE_CLK board_cpu_freq()
#define SYSTICK_IRQ_PRI (14)

void sys_tick_init(void);
uint32_t get_sys_tick();

void update_systick(uint32_t ticks);
int get_time_gap(uint32_t t);

void mark_start_tick();
void mark_enter_idle();
void mark_exit_idle();
int get_idle_percentage();
#endif //_SYS_TICK_H_

