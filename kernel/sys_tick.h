#ifndef _SYS_TICK_H_
#define _SYS_TICK_H_

#include <stdint.h>

#define SYSTICK_HZ  (1000)
#define CORE_CLK board_cpu_freq()
#define SYSTICK_IRQ_PRI (14)

void sys_tick_init(void);
uint32_t get_sys_tick();

#endif //_SYS_TICK_H_

