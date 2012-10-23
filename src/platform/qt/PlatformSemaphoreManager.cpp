#include "PlatformSemaphoreManager.h"

#include "PlatformThread.h"
#include "Referenceable.h"


class PlatformSemaphoreManager::Semaphore : public Referenceable {
public:
	Semaphore(int32 count, const char* name)
		:
		fMutex(),
		fName(name),
		fCount(count),
		fWaiters()
	{
	}

	~Semaphore()
	{
	}

	void Delete()
	{
		QMutexLocker mutexLocker(&fMutex);
		while (Waiter* waiter = fWaiters.GetHead()) {
			fWaiters.Remove(waiter);
			waiter->waitStatus = B_BAD_SEM_ID;
			waiter->thread->Unblock();
		}
	}

	status_t Acquire(int32 count, uint32 flags, bigtime_t timeout)
	{
		if (count <= 0)
			return B_BAD_VALUE;

		QMutexLocker mutexLocker(&fMutex);
		if (count <= fCount) {
			fCount -= count;
			return B_OK;
		}

		if ((flags & B_RELATIVE_TIMEOUT) != 0 && timeout == 0)
			return B_WOULD_BLOCK;

		if (fWaiters.IsEmpty())
			fCount -= count;

		Waiter waiter;
		waiter.thread = PlatformThread::CurrentThread();
		waiter.count = count;
		waiter.waitStatus = WAITING;
		fWaiters.Insert(&waiter);

		waiter.thread->PrepareToBlock();
		mutexLocker.unlock();
		waiter.thread->Block();
		mutexLocker.relock();

		if (waiter.waitStatus == WAITING) {
			bool isFirst = fWaiters.GetHead() == &waiter;
			fWaiters.Remove(&waiter);
			if (isFirst)
				_Release(count);
		}

		return waiter.waitStatus == WAITING ? B_TIMED_OUT : waiter.waitStatus;
	}

	status_t Release(int32 count, uint32 flags)
	{
		if (count <= 0)
			return B_BAD_VALUE;

		QMutexLocker mutexLocker(&fMutex);
		_Release(count);
		return B_OK;
	}

private:
	struct Waiter : DLListLinkImpl<Waiter> {
		PlatformThread*	thread;
		int32			count;
		status_t		waitStatus;
	};

	typedef DLList<Waiter> WaiterList;

	static const status_t WAITING = 1;

private:
	void _Release(int32 count)
	{
		fCount += count;
		while (fCount >= 0 && !fWaiters.IsEmpty()) {
			Waiter* waiter = fWaiters.GetHead();
			fWaiters.Remove(waiter);
			waiter->waitStatus = B_OK;
			waiter->thread->Unblock();

			if (Waiter* nextWaiter = fWaiters.GetHead())
				fCount -= nextWaiter->count;
		}
	}

private:
	QMutex		fMutex;
	BString		fName;
	int32		fCount;
	WaiterList	fWaiters;
};


/*static*/PlatformSemaphoreManager*
PlatformSemaphoreManager::Manager()
{
	static PlatformSemaphoreManager* manager = new PlatformSemaphoreManager;
	return manager;
}


sem_id
PlatformSemaphoreManager::CreateSemaphore(int32 count, const char* name)
{
	QMutexLocker mutexLocker(&fMutex);
	try {
		Semaphore* semaphore = new Semaphore(count, name);
		Reference<Semaphore> semaphoreReference(semaphore, true);
		sem_id id = fNextSemaphoreId++;
		fSemaphores.insert(id, semaphore);
		semaphoreReference.Detach();
		return id;
	} catch (std::bad_alloc) {
		return B_NO_MEMORY;
	}
}


status_t
PlatformSemaphoreManager::DeleteSemaphore(sem_id id)
{
	QMutexLocker mutexLocker(&fMutex);
	Semaphore* semaphore = fSemaphores.value(id, NULL);
	if (semaphore == NULL)
		return B_BAD_SEM_ID;
	mutexLocker.unlock();

	semaphore->Delete();
	semaphore->RemoveReference();
	return B_OK;
}


status_t
PlatformSemaphoreManager::AcquireSemaphore(sem_id id, int32 count, uint32 flags,
	bigtime_t timeout)
{
	QMutexLocker mutexLocker(&fMutex);
	Semaphore* semaphore = fSemaphores.value(id, NULL);
	if (semaphore == NULL)
		return B_BAD_SEM_ID;
	Reference<Semaphore> semaphoreReference(semaphore);
	mutexLocker.unlock();

	return semaphore->Acquire(count, flags, timeout);
}


status_t
PlatformSemaphoreManager::ReleaseSemaphore(sem_id id, int32 count,
	uint32 flags)
{
	QMutexLocker mutexLocker(&fMutex);
	Semaphore* semaphore = fSemaphores.value(id, NULL);
	if (semaphore == NULL)
		return B_BAD_SEM_ID;
	Reference<Semaphore> semaphoreReference(semaphore);
	mutexLocker.unlock();

	return semaphore->Release(count, flags);
}


PlatformSemaphoreManager::PlatformSemaphoreManager()
	:
	fMutex(),
	fSemaphores(),
	fNextSemaphoreId(1)
{
}
