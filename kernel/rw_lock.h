#ifndef _RW_LOCK_H_
#define _RW_LOCK_H_

#include "mutex.h"
#include "semaphore.h"

struct rw_lock {

	/** Mutex for protect reader_count */
	struct mutex reader_mutex;

	unsigned int reader_count;

    /* key component */
	struct semaphore rw_lock;
	
};


#endif //_RW_LOCK_H_
