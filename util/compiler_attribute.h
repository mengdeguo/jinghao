#ifndef _COMPILER_ATTRIBUTE_H_
#define _COMPILER_ATTRIBUTE_H_


#ifndef __ALIGN
    #define __ALIGN(n)      __attribute__((aligned(n)))
#endif

#ifndef __PACKED
    #define __PACKED        __attribute__((packed)) 
#endif

#endif //_COMPILER_ATTRIBUTE_H_
