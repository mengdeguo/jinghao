#ifndef _KERNEL_CONFIG_H_
#define _KERNEL_CONFIG_H_

/* UTIL */
#define ENABLE(feature) (defined (ENABLE_##feature)  && (ENABLE_##feature) )

/* feature control */
#define ENABLE_CALCULATE_CPU_LOAD  1

#endif //_KERNEL_CONFIG_H_
