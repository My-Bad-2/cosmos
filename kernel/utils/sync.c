#include <arch.h>
#include <utils/sync.h>

#define SYNC_FETCH_ADD(object, operand, order)                                 \
	__c11_atomic_fetch_add(object, operand, order)
#define SYNC_LOAD(object, order) __c11_atomic_load(object, order)
#define SYNC_STORE(object, desired, order)                                     \
	__c11_atomic_store(object, desired, order)

void lock_acquire(lock_t* lock) {
	size_t ticket = SYNC_FETCH_ADD(&lock->next_ticket, 1, memory_order_relaxed);

	while (SYNC_LOAD(&lock->serving_ticket, memory_order_acquire) != ticket) {
		arch_pause();
	}
}

void lock_release(lock_t* lock) {
	if (!lock_is_locked(lock)) {
		return;
	}

	size_t current = SYNC_LOAD(&lock->serving_ticket, memory_order_relaxed);
	SYNC_STORE(&lock->serving_ticket, current + 1, memory_order_release);
}

bool lock_is_locked(lock_t* lock) {
	size_t current = SYNC_LOAD(&lock->serving_ticket, memory_order_relaxed);
	size_t next = SYNC_LOAD(&lock->next_ticket, memory_order_relaxed);

	return current != next;
}

bool try_lock(lock_t* lock) {
	if (lock_is_locked(lock)) {
		return false;
	}

	lock_acquire(lock);
	return true;
}