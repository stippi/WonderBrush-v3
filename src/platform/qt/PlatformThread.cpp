#include "PlatformThread.h"

#include <QMutexLocker>
#include <QThread>
#include <QThreadStorage>


PlatformThread::PlatformThread(thread_id id)
	:
	fId(id),
	fMutex(),
	fBlockCondition(),
	fBlocked(false),
	fWaiting(false)
{
}


void
PlatformThread::PrepareToBlock()
{
	QMutexLocker mutexLocker(&fMutex);
	fBlocked = true;
}


void
PlatformThread::Block()
{
	QMutexLocker mutexLocker(&fMutex);
	if (fBlocked) {
		fWaiting = true;
		fBlockCondition.wait(&fMutex);
		fWaiting = false;
	}
}


bool
PlatformThread::BlockWithTimeout(uint32 flags, bigtime_t timeout)
{
	if ((flags & (B_RELATIVE_TIMEOUT | B_ABSOLUTE_TIMEOUT)) == 0) {
		Block();
		return true;
	}

	bigtime_t absoluteTimeout = timeout;
	if ((flags & B_RELATIVE_TIMEOUT) != 0)
		absoluteTimeout += system_time();

	QMutexLocker mutexLocker(&fMutex);

	if (!fBlocked)
		return true;

	if (timeout <= 0) {
		fBlocked = false;
		return false;
	}

	for (;;) {
		bigtime_t timeoutMS = timeout / 1000;
		fWaiting = true;
		fBlockCondition.wait(&fMutex,
			timeoutMS <= INT_MAX ? (int)timeoutMS : INT_MAX);
		fWaiting = false;

		if (!fBlocked)
			break;

		timeout = absoluteTimeout - system_time();
		if (timeout <= 0)
			break;
	}

	if (fBlocked) {
		fBlocked = false;
		return false;
	}

	return true;
}


void
PlatformThread::Unblock()
{
	QMutexLocker mutexLocker(&fMutex);
	if (fBlocked) {
		fBlocked = false;
		if (fWaiting) {
			fWaiting = false;
			fBlockCondition.wakeOne();
		}
	}
}

PlatformThread*
PlatformThread::CurrentThread()
{
	static QThreadStorage<PlatformThread*> threads;
	static vint32 nextThreadId = 1;

	if (threads.hasLocalData())
		return threads.localData();

	PlatformThread* thread = new PlatformThread(atomic_add(&nextThreadId, 1));
	threads.setLocalData(thread);
	return thread;
}
