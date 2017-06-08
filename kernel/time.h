#ifndef _TIME_H_
#define _TIME_H_

#include <stdint.h>

void delay_ms(uint32_t ms);
void delay_ticks_procedure(void);
void add_tcb_to_wait_list(tcb * tcb);
void remove_tcb_from_wait_list(tcb * tcb);

#endif //_TIME_H_
