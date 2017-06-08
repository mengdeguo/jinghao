#include "kernel_header.h"
#include "os_debug.h"

#if ENABLE(RTOS_DEBUG)

void schedule_to_thread(tcb * t)
{
    OS_LOG("switch to : %s \r\n",t->name);
}

#endif 

