/*
 * Copyright 2001-2010, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef	_LOCKER_H
#define	_LOCKER_H


#include <OS.h>
#include <String.h>

#include <QMutex>


class BLocker {
public:
								BLocker();
								BLocker(const char* name);
								BLocker(bool benaphoreStyle);
								BLocker(const char* name, bool benaphoreStyle);
	virtual						~BLocker();

			status_t			InitCheck() const;

			bool				Lock();
			status_t			LockWithTimeout(bigtime_t timeout);
			void				Unlock();

			thread_id			LockingThread() const;
			bool				IsLocked() const;
			int32				CountLocks() const;
//			int32				CountLockRequests() const;
//			sem_id				Sem() const;

private:
			void				InitLocker(const char* name,
									bool benaphoreStyle);

private:
			BString				fName;
			QMutex				fMutex;
			thread_id			fLockOwner;
			int32				fRecursiveCount;
};


#endif	// _LOCKER_H
