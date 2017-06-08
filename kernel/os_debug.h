#ifndef _OS_DEBUG_H_
#define _OS_DEBUG_H_

#include "platform.h"

#if ENABLE(RTOS_DEBUG)

void schedule_to_thread(tcb * t);

#endif

#endif //_OS_DEBUG_H_
