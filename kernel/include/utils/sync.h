#ifndef UTILS_SYNC_H
#define UTILS_SYNC_H

#include <stdatomic.h>
#include <stdbool.h>

#define LOCK_INITIALIZER ((lock_t){0, 0})

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	atomic_size_t next_ticket;
	atomic_size_t serving_ticket;
} lock_t;

void lock_acquire(lock_t* lock);
void lock_release(lock_t* lock);
bool lock_is_locked(lock_t* lock);
bool try_lock(lock_t* lock);

#ifdef __cplusplus
}
#endif

#endif	// UTILS_SYNC_H