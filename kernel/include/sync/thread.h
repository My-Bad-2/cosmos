#ifndef SYNC_THREAD_H
#define SYNC_THREAD_H

#include "sync/arch_thread.h"
#include <stdint.h>
#include <sys/types.h>
#include <utils/list.h>

#define THREAD_FLAG_DETACHED (1 << 0)
#define THREAD_FLAG_FREE_STACK (1 << 1)
#define THREAD_FLAG_FREE_STRUCT (1 << 2)
#define THREAD_FLAG_REAL_TIME (1 << 3)
#define THREAD_FLAG_IDLE (1 << 4)

#define THREAD_MAGIC (0x74687264)

enum thread_tls_list { MAX_TLS_ENTRY };

enum thread_state {
	THREAD_SUSPENDED,
	THREAD_READY,
	THREAD_RUNNING,
	THREAD_BLOCKED,
	THREAD_SLEEPING,
	THREAD_DEATH,
};

typedef int (*thread_start_routine)(void* arg);

struct thread_specific_stats {
	time_t total_run_time;
	time_t last_run_timestamp;
	size_t schedules;
};

struct thread {
	int magic;

	struct list_node thread_list_node;
	struct list_node queue_node;

	int priority;
	enum thread_state state;

	int remaining_quantum;
	uint32_t flags;

	struct pte_table* pagemap;
	struct arch_thread arch;

	void* stack;
	size_t stack_size;

	thread_start_routine entry;
	void* arg;

	int ret_code;

	uintptr_t tls[MAX_TLS_ENTRY];

	char name[32];

	struct thread_specific_stats stats;
};

#endif	// SYNC_THREAD_H