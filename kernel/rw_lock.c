#include "kernel_header.h"

/* with reader callback */
void rwlock_init_with_rcb(struct rw_lock * lock, rw_lock_rcb rcb)
{
    mutex_init(&lock->reader_mutex,RECU_NORMAL);
	semaphore_init(&lock->rw_lock,1);

	lock->reader_count = 0;
    lock->rcb = rcb;
}

void rwlock_init(struct rw_lock *lock)
{
    rwlock_init_with_rcb(lock,NULL);
}
   
int rwlock_read_lock(struct rw_lock *lock, uint32_t wait_time)
{
	int ret = 0;

	ret = mutex_lock(&lock->reader_mutex);
	if (ret) {
		return ret;
	}

	lock->reader_count++;
	if (lock->reader_count == 1) {
        
        if(lock->rcb) {
            ret = lock->rcb(lock,wait_time);
            if(ret) {
                lock->reader_count--;
                mutex_unlock(&lock->reader_mutex);

                return ret;
            }
        } else {
		
            ret = down_semaphore(&lock->rw_lock,wait_time);
            if (ret == ERR_FAIL) {
                lock->reader_count--;
                mutex_unlock(&lock->reader_mutex);
                return ret;
            }
        }
	}

	mutex_unlock(&lock->reader_mutex);

	return ret;
}

int rwlock_read_unlock(struct rw_lock *lock)
{
	int ret = mutex_lock(&lock->reader_mutex);
	if (ret)
		return ret;

	lock->reader_count--;
	if (lock->reader_count == 0) {
		
        up_semaphore(&lock->rw_lock);
	}

	mutex_unlock(&lock->reader_mutex);

	return ret;
}

int rwlock_write_lock(struct rw_lock *lock,uint32_t wait_time)
{
	return down_semaphore(&lock->rw_lock, wait_time);
}

void rwlock_write_unlock(struct rw_lock *lock)
{
	up_semaphore(&lock->rw_lock);
}

void rwlock_release(struct rw_lock *lock)
{
	semaphore_release(&lock->rw_lock);
	mutex_release(&lock->reader_mutex);
	lock->reader_count = 0;
    lock->rcb = NULL;
}

