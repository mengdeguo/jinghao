#ifndef _TIME_H_
#define _TIME_H_

#include <stdint.h>
#include "sys_tick.h"
#include "thread.h"


#define MS_TO_TICKS(ms) ((ms) * SYSTICK_HZ / 1000 )

void delay_ms(uint32_t ms);
void delay_ticks_procedure(void);
void add_tcb_to_wait_list(tcb * tcb);
void remove_tcb_from_wait_list(tcb * tcb);

#endif //_TIME_H_
