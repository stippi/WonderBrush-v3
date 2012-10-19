#ifndef PLATFORM_THREAD_H
#define PLATFORM_THREAD_H


#include <OS.h>

#include <QMutex>
#include <QObject>
#include <QWaitCondition>

#include "DLList.h"
#include "Referenceable.h"


class PlatformThread : public QObject, public Referenceable,
	public DLListLinkImpl<PlatformThread> {
public:
								PlatformThread(thread_id id, bool running);
								~PlatformThread();

			thread_id			Id() const	{ return fId; }

			void				PrepareToBlock();
			void				Block();
			bool				BlockWithTimeout(uint32 flags,
									bigtime_t timeout);
			void				Unblock();

	static	PlatformThread*		CurrentThread();
	static	thread_id			CurrentThreadId()
									{ return CurrentThread()->Id(); }
	static	PlatformThread*		ThreadById(thread_id id);

	static	PlatformThread*		Spawn(thread_func function, const char* name,
									int32 priority, void* data);
	static	status_t			Resume(thread_id id);
	static	status_t			WaitFor(thread_id id, status_t* _returnValue);

private:
			struct Thread;
			struct Manager;
			struct Handle;

private:
			void				_UnblockLocked();
			void				_Resume();

	static	thread_id			_NextThreadID();

private:
			thread_id			fId;
			Thread*				fThread;
			QMutex				fMutex;
			QWaitCondition		fBlockCondition;
			bool				fBlocked;
			bool				fWaiting;
			bool				fRunning;
			status_t			fExitValue;
};


#endif // PLATFORM_THREAD_H
