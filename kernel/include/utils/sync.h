#ifndef UTILS_SYNC_H
#define UTILS_SYNC_H

#include <stdatomic.h>
#include <stdbool.h>

#define LOCK_INITIALIZER ((struct lock){0, 0})

#ifdef __cplusplus
extern "C" {
#endif

struct lock {
	atomic_size_t next_ticket;
	atomic_size_t serving_ticket;
};

void lock_acquire(struct lock* lock);
void lock_release(struct lock* lock);
bool lock_is_locked(struct lock* lock);
bool try_lock(struct lock* lock);

#ifdef __cplusplus
}
#endif

#endif	// UTILS_SYNC_H