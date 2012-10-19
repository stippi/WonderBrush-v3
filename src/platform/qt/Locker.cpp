#include <Locker.h>


BLocker::BLocker()
{
	InitLocker(NULL, true);
}


BLocker::BLocker(const char *name)
{
	InitLocker(name, true);
}


BLocker::BLocker(bool benaphoreStyle)
{
	InitLocker(NULL, benaphoreStyle);
}


BLocker::BLocker(const char *name, bool benaphoreStyle)
{
	InitLocker(name, benaphoreStyle);
}


BLocker::~BLocker()
{
}


status_t
BLocker::InitCheck() const
{
	return B_OK;
}


bool
BLocker::Lock()
{
	fMutex.lock();
	if (++fRecursiveCount == 1)
		fLockOwner = find_thread(NULL);
	return B_OK;
}


status_t
BLocker::LockWithTimeout(bigtime_t timeout)
{
	if (timeout <= 0) {
		if (!fMutex.tryLock())
			return B_WOULD_BLOCK;
	} else {
		bigtime_t absoluteTimeout = system_time() + timeout;
		for (;;) {
			bigtime_t timeoutMS = timeout / 1000;
			if (fMutex.tryLock(timeoutMS <= INT_MAX ? (int)timeoutMS : INT_MAX))
				break;

			timeout = absoluteTimeout - system_time();
			if (timeout <= 0)
				return B_TIMED_OUT;
		}
	}

	if (++fRecursiveCount == 1)
		fLockOwner = find_thread(NULL);
	return B_OK;
}


void
BLocker::Unlock()
{
	if (IsLocked()) {
		if (--fRecursiveCount == 0)
			fLockOwner = -1;

		fMutex.unlock();
    }
}


thread_id
BLocker::LockingThread() const
{
    return fLockOwner;
}


bool
BLocker::IsLocked() const
{
	return fRecursiveCount > 0 && find_thread(NULL) == fLockOwner;
}


int32
BLocker::CountLocks() const
{
    return fRecursiveCount;
}


void
BLocker::InitLocker(const char *name, bool benaphore)
{
	if (name == NULL)
		name = "some BLocker";

	fName = name;
	fLockOwner = -1;
	fRecursiveCount = 0;
}
