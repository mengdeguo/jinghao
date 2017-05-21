#ifndef _BASE_INC_H_
#define _BASE_INC_H_

#include <stddef.h>

/* utility */
// file stddef.h: contains the defination of offsetof 
//#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
//#define offsetof(typ, memb)     ((unsigned long)((char *)&(((typ *)0)->memb)))

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})





#endif // _BASE_INC_H_
