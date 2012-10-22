#include "PlatformThread.h"

#include <QMap>
#include <QMutexLocker>
#include <QThread>
#include <QThreadStorage>

#include <OS.h>


struct PlatformThread::Thread : public QThread {
	Thread(PlatformThread* thread, thread_func function, const char* name,
		int32 priority, void* data)
		:
		QThread(thread),
		fPlatformThread(thread),
		fFunction(function),
		fData(data)
	{
		fPlatformThread->fThread = this;
		// TODO: Set priority!
	}

	PlatformThread* GetPlatformThread() const
	{
		return fPlatformThread;
	}

	virtual void run()
	{
		// Make sure we are registered with the QThreadStorage, so that the
		// PlatformThread object is dereferenced when the thread terminates.
		CurrentThread();

		// invoke the actual thread function
		fPlatformThread->fExitValue = fFunction(fData);
	}

private:
	PlatformThread*	fPlatformThread;
	thread_func		fFunction;
	void*			fData;
};


struct PlatformThread::Manager {
	Manager()
		:
		fMutex(),
		fThreads()
	{
	}

	static Manager* GetManager()
	{
		static Manager manager;
		return &manager;
	}

	void RegisterThread(PlatformThread* thread)
	{
		QMutexLocker mutexLocker(&fMutex);
		fThreads.insert(thread->Id(), thread);
	}

	void UnregisterThread(PlatformThread* thread)
	{
		QMutexLocker mutexLocker(&fMutex);
		fThreads.remove(thread->Id());
	}

	PlatformThread* ThreadByID(thread_id id)
	{
		QMutexLocker mutexLocker(&fMutex);
		return fThreads.value(id, NULL);
	}

	Reference<PlatformThread> ThreadReferenceByID(thread_id id)
	{
		QMutexLocker mutexLocker(&fMutex);
		return fThreads.value(id, NULL);
	}

	status_t ResumeThread(thread_id id)
	{
		QMutexLocker mutexLocker(&fMutex);
		PlatformThread* thread = fThreads.value(id, NULL);
		if (thread == NULL)
			return B_BAD_THREAD_ID;

		thread->_Resume();
		return B_OK;
	}

	status_t WaitFor(thread_id id, status_t* _returnValue)
	{
		// If the thread isn't running yet, resume it. We make the call
		// unconditionally -- it's a no-op, if the thread is already running.
		ResumeThread(id);

		Reference<PlatformThread> threadReference = ThreadReferenceByID(id);
		PlatformThread* thread = threadReference.Get();
		if (thread == NULL)
			return B_BAD_THREAD_ID;

		// If we haven't created the thread we cannot wait for it, since we
		// don't control the life time of the QThread object and thus cannot
		// safely invoke methods on it.
		if (thread->fThread == NULL) {
			debugger("PlatformThread::Manager::WaitFor(): Can't wait for "
				"thread we haven't created!");
			return B_BAD_VALUE;
		}

		thread->fThread->wait();

		if (_returnValue != NULL)
			*_returnValue = thread->fExitValue;

		return B_OK;
	}

private:
	QMutex								fMutex;
	QMap<thread_id, PlatformThread*>	fThreads;
};


struct PlatformThread::Handle {
	Handle(PlatformThread* thread)
		:
		fThread(thread)
	{
	}

	~Handle()
	{
		fThread->RemoveReference();
	}

	PlatformThread* GetThread() const
	{
		return fThread;
	}

private:
	PlatformThread*	fThread;
};


PlatformThread::PlatformThread(thread_id id, bool running)
	:
	QObject(),
	fId(id),
	fThread(NULL),
	fMutex(),
	fBlockCondition(),
	fBlocked(false),
	fWaiting(false),
	fRunning(running),
	fExitValue(B_OK)
{
	Manager::GetManager()->RegisterThread(this);
}


PlatformThread::~PlatformThread()
{
	Manager::GetManager()->UnregisterThread(this);
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
	_UnblockLocked();
}


PlatformThread*
PlatformThread::CurrentThread()
{
	static QThreadStorage<PlatformThread::Handle*> threads;

	if (threads.hasLocalData())
		return threads.localData()->GetThread();

	// If this code created the thread, there will be a Thread object.
	if (Thread* thread = dynamic_cast<Thread*>(QThread::currentThread())) {
		threads.setLocalData(new Handle(thread->GetPlatformThread()));
		return thread->GetPlatformThread();
	}

	// Someone else created the thread. Just create a new PlatformThread object.
	PlatformThread* platformThread = new PlatformThread(_NextThreadID(), true);
	threads.setLocalData(new Handle(platformThread));
	return platformThread;
}


PlatformThread*
PlatformThread::ThreadById(thread_id id)
{
	return Manager::GetManager()->ThreadByID(id);
}


/*static*/ PlatformThread*
PlatformThread::Spawn(thread_func function, const char* name, int32 priority,
	void* data)
{
	PlatformThread* platformThread = new PlatformThread(_NextThreadID(), false);

	new Thread(platformThread, function, name, priority, data);

	return platformThread;
}


/*static*/ status_t
PlatformThread::Resume(thread_id id)
{
	return Manager::GetManager()->ResumeThread(id);
}


/*static*/ status_t
PlatformThread::WaitFor(thread_id id, status_t* _returnValue)
{
	return Manager::GetManager()->WaitFor(id, _returnValue);
}


void
PlatformThread::_UnblockLocked()
{
	if (fBlocked) {
		fBlocked = false;
		if (fWaiting) {
			fWaiting = false;
			fBlockCondition.wakeOne();
		}
	}
}


void
PlatformThread::_Resume()
{
	QMutexLocker mutexLocker(&fMutex);
	if (!fRunning) {
		fRunning = true;
		fThread->start();
	}
}


thread_id
PlatformThread::_NextThreadID()
{
	static vint32 nextThreadId = 1;
	return atomic_add(&nextThreadId, 1);
}
