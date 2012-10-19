#ifndef OS_H
#define OS_H


#include <SupportDefs.h>


typedef int32 thread_id;
typedef int32 team_id;
typedef int32 sem_id;


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

bigtime_t system_time(void);
status_t snooze(bigtime_t amount);
status_t snooze_until(bigtime_t time, int timeBase);

thread_id find_thread(const char* name);

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


#ifdef __cplusplus
}
#endif


#endif // OS_H
