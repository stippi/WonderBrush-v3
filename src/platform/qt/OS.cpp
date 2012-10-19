#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <sys/stat.h>
#include <sys/time.h>

//#include <Debug.h>
#include <OS.h>

#include <QThread>

#include "PlatformSemaphoreManager.h"
#include "PlatformThread.h"


mode_t __gUmask = 022;


// debugger
void
debugger(const char *message)
{
	fprintf(stderr, "debugger() called: %s\n", message);
	exit(1);
}


// _debuggerAssert
//int
//_debuggerAssert(const char *file, int line, const char *expression)
//{
//	char buffer[2048];
//	snprintf(buffer, sizeof(buffer), "%s:%d: %s\n", file, line, expression);
//	debugger(buffer);
//	return 0;
//}


// system_time
bigtime_t
system_time(void)
{
	struct timeval tm;
	gettimeofday(&tm, NULL);
	return (int64)tm.tv_sec * 1000000LL + (int64)tm.tv_usec;
}


sem_id
create_sem(int32 count, const char* name)
{
	return PlatformSemaphoreManager::Manager()->CreateSemaphore(count, name);
}


status_t
delete_sem(sem_id id)
{
	return PlatformSemaphoreManager::Manager()->DeleteSemaphore(id);
}


status_t
acquire_sem(sem_id id)
{
	return PlatformSemaphoreManager::Manager()->AcquireSemaphore(id);
}


status_t
acquire_sem_etc(sem_id id, int32 count, uint32 flags, bigtime_t timeout)
{
	return PlatformSemaphoreManager::Manager()->AcquireSemaphore(id, count,
		flags, timeout);
}


status_t
release_sem(sem_id id)
{
	return PlatformSemaphoreManager::Manager()->ReleaseSemaphore(id);
}


status_t
release_sem_etc(sem_id id, int32 count, uint32 flags)
{
	return PlatformSemaphoreManager::Manager()->ReleaseSemaphore(id, count,
		flags);
}


thread_id
spawn_thread(thread_func function, const char* name, int32 priority, void* data)
{
	return PlatformThread::Spawn(function, name, priority, data)->Id();
}


status_t
resume_thread(thread_id thread)
{
	return PlatformThread::Resume(thread);
}


status_t
wait_for_thread(thread_id thread, status_t* _returnValue)
{
	return PlatformThread::WaitFor(thread, _returnValue);
}


thread_id
find_thread(const char* name)
{
	// TODO: We could support finding threads by name.
	return name == NULL ? PlatformThread::CurrentThreadId() : B_NAME_NOT_FOUND;
}


// snooze
status_t
snooze(bigtime_t amount)
{
	// NOTE: It would be nice to just use QThread::usleep(), but for some
	// reason it is protected.
	if (amount <= 0)
		return B_OK;

	int64 secs = amount / 1000000LL;
	int64 usecs = amount % 1000000LL;
	if (secs > 0) {
//		if (sleep((unsigned)secs) < 0)
//			return errno;
		sleep((unsigned)secs);
	}

	if (usecs > 0) {
		if (usleep((useconds_t)usecs) < 0)
			return errno;
	}

	return B_OK;
}

// snooze_until
status_t
snooze_until(bigtime_t time, int timeBase)
{
	return snooze(time - system_time());
}
