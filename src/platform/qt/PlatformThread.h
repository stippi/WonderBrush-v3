#ifndef PLATFORM_THREAD_H
#define PLATFORM_THREAD_H


#include <OS.h>

#include <QMutex>
#include <QWaitCondition>

#include "DLList.h"


class PlatformThread : public DLListLinkImpl<PlatformThread> {
public:
								PlatformThread(thread_id id);

			thread_id			Id() const	{ return fId; }

			void				PrepareToBlock();
			void				Block();
			bool				BlockWithTimeout(uint32 flags,
									bigtime_t timeout);
			void				Unblock();

	static	PlatformThread*		CurrentThread();
	static	thread_id			CurrentThreadId()
									{ return CurrentThread()->Id(); }

private:
			thread_id			fId;
			QMutex				fMutex;
			QWaitCondition		fBlockCondition;
			bool				fBlocked;
			bool				fWaiting;
};


#endif // PLATFORM_THREAD_H
