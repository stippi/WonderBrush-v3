/*
 * Copyright 2004-2009, Haiku Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef PLATFORM_QT_OS_H
#define PLATFORM_QT_OS_H


#include <SupportDefs.h>


typedef int32 area_id;
typedef int32 port_id;
typedef int32 sem_id;
typedef int32 team_id;
typedef int32 thread_id;


#define B_INFINITE_TIMEOUT      (9223372036854775807LL)

enum {
	B_TIMEOUT			= 0x8,  /* relative timeout */
	B_RELATIVE_TIMEOUT	= 0x8,  /* fails after a relative timeout
								   with B_TIMED_OUT */
	B_ABSOLUTE_TIMEOUT	= 0x10 /* fails after an absolute timeout
								   with B_TIMED_OUT */

	/* experimental Haiku only API */
//	B_TIMEOUT_REAL_TIME_BASE		= 0x40,
//	B_ABSOLUTE_REAL_TIME_TIMEOUT	= B_ABSOLUTE_TIMEOUT
//										| B_TIMEOUT_REAL_TIME_BASE
};


#ifdef __cplusplus
extern "C" {
#endif


void debugger(const char *message);

bigtime_t	system_time(void);


/* semaphore flags */
enum {
	B_CAN_INTERRUPT				= 0x01,	/* acquisition of the semaphore can be
										   interrupted (system use only) */
	B_CHECK_PERMISSION			= 0x04,	/* ownership will be checked (system use
										   only) */
	B_KILL_CAN_INTERRUPT		= 0x20,	/* acquisition of the semaphore can be
										   interrupted by SIGKILL[THR], even
										   if not B_CAN_INTERRUPT (system use
										   only) */

	/* release_sem_etc() only flags */
	B_DO_NOT_RESCHEDULE			= 0x02	/* thread is not rescheduled */
//	B_RELEASE_ALL				= 0x08,	/* all waiting threads will be woken up,
//										   count will be zeroed */
//	B_RELEASE_IF_WAITING_ONLY	= 0x10	/* release count only if there are any
//										   threads waiting */
};

sem_id		create_sem(int32 count, const char* name);
status_t	delete_sem(sem_id id);
status_t	acquire_sem(sem_id id);
status_t	acquire_sem_etc(sem_id id, int32 count, uint32 flags,
				bigtime_t timeout);
status_t	release_sem(sem_id id);
status_t	release_sem_etc(sem_id id, int32 count, uint32 flags);
/* TODO: the following two calls are not part of the BeOS API, and might be
   changed or even removed for the final release of Haiku R1 */
//status_t	switch_sem(sem_id semToBeReleased, sem_id id);
//status_t	switch_sem_etc(sem_id semToBeReleased, sem_id id,
//					int32 count, uint32 flags, bigtime_t timeout);
status_t	get_sem_count(sem_id id, int32* threadCount);
status_t	set_sem_owner(sem_id id, team_id team);

status_t	get_sem_info(sem_id id, struct sem_info* info);
status_t	get_next_sem_info(team_id team, int32* cookie,
				struct sem_info* info);


/* Threads */

#define B_IDLE_PRIORITY					0
#define B_LOWEST_ACTIVE_PRIORITY		1
#define B_LOW_PRIORITY					5
#define B_NORMAL_PRIORITY				10
#define B_DISPLAY_PRIORITY				15
#define	B_URGENT_DISPLAY_PRIORITY		20
#define	B_REAL_TIME_DISPLAY_PRIORITY	100
#define	B_URGENT_PRIORITY				110
#define B_REAL_TIME_PRIORITY			120

#define B_SYSTEM_TIMEBASE				0
	/* time base for snooze_*(), compatible with the clockid_t constants defined
	   in <time.h> */

#define B_FIRST_REAL_TIME_PRIORITY		B_REAL_TIME_DISPLAY_PRIORITY

typedef status_t (*thread_func)(void *);
#define thread_entry thread_func
	/* thread_entry is for backward compatibility only! Use thread_func */

thread_id	spawn_thread(thread_func, const char *name, int32 priority,
				void *data);
status_t	kill_thread(thread_id thread);
status_t	resume_thread(thread_id thread);
status_t	suspend_thread(thread_id thread);

status_t	rename_thread(thread_id thread, const char *newName);
status_t	set_thread_priority(thread_id thread, int32 newPriority);
void		exit_thread(status_t status);
status_t	wait_for_thread(thread_id thread, status_t *returnValue);
status_t	on_exit_thread(void (*callback)(void *), void *data);

thread_id 	find_thread(const char *name);

status_t	snooze(bigtime_t amount);
status_t	snooze_etc(bigtime_t amount, int timeBase, uint32 flags);
status_t	snooze_until(bigtime_t time, int timeBase);


#ifdef __cplusplus
}
#endif


#endif // PLATFORM_QT_OS_H
