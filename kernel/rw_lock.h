#ifndef _RW_LOCK_H_
#define _RW_LOCK_H_

#include <stdint.h>

struct rw_lock;

typedef int (*rw_lock_rcb) (struct rw_lock *plock, uint32_t wait_time);

struct rw_lock {

	/** Mutex for protect reader_count */
	struct mutex    reader_mutex;

	unsigned int    reader_count;
    rw_lock_rcb      rcb;         /*read callback*/

    /* key component */
	struct semaphore rw_lock;
};

void rwlock_init_with_rcb(struct rw_lock * lock, rw_lock_rcb rcb);
void rwlock_init(struct rw_lock *lock);
int rwlock_read_lock(struct rw_lock *lock, uint32_t wait_time);
int rwlock_read_unlock(struct rw_lock *lock);
int rwlock_write_lock(struct rw_lock *lock,uint32_t wait_time);
void rwlock_write_unlock(struct rw_lock *lock);
void rwlock_release(struct rw_lock *lock);

#endif //_RW_LOCK_H_
