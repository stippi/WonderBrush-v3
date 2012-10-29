/*
 * Copyright 2001-2008, Haiku Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Erik Jaesler (erik@cgsoftware.com)
 */
#ifndef PLATFORM_QT_B_LOOPER_H
#define PLATFORM_QT_B_LOOPER_H


#include <Handler.h>

#include <QHash>
#include <QList>
#include <QSet>

#include "Referenceable.h"


// Port (Message Queue) Capacity
#define B_LOOPER_PORT_DEFAULT_CAPACITY	200


class BMessageFilter;
class BList;
class BView;


class BLooper : public BHandler {
public:
			struct MessageTarget;
			struct Manager;

public:
								BLooper(const char* name = NULL,
									int32 priority = B_NORMAL_PRIORITY,
									int32 port_capacity
										= B_LOOPER_PORT_DEFAULT_CAPACITY);
	virtual						~BLooper();

	// Message transmission
			status_t			PostMessage(uint32 command);
			status_t			PostMessage(BMessage* message);
			status_t			PostMessage(uint32 command, BHandler* handler,
									BHandler* replyTo = NULL);
			status_t			PostMessage(BMessage* message,
									BHandler* handler,
									BHandler* replyTo = NULL);

	virtual	void				DispatchMessage(BMessage* message,
									BHandler* handler);

	// Message handlers
			void				AddHandler(BHandler* handler);
			bool				RemoveHandler(BHandler* handler);
			int32				CountHandlers() const;
			BHandler*			HandlerAt(int32 index) const;
			int32				IndexOf(BHandler* handler) const;

			BHandler*			PreferredHandler() const;
			void				SetPreferredHandler(BHandler* handler);

			BMessage*			CurrentMessage() const
									{ return fLastMessage; }

			bool				Lock();
			void				Unlock();
			bool				IsLocked() const;
			status_t			LockWithTimeout(bigtime_t timeout);

	virtual	void				AddCommonFilter(BMessageFilter* filter);
	virtual	bool				RemoveCommonFilter(BMessageFilter* filter);
	virtual	void				SetCommonFilterList(BList* filters);
			BList*				CommonFilterList() const;

			MessageTarget*		GetMessageTarget() const
									{ return fMessageTarget; }

protected:
			void				ObjectConstructed(QObject* object);
			void				ObjectAboutToBeDestroyed(QObject* object);

private:
			struct LooperLock;

			friend class BView;

private:
			void				_InitData(const char* name, int32 priority);

			status_t			_Lock(bigtime_t timeout);
			bool				AssertLocked() const;

			BHandler*			_TopLevelFilter(BMessage* message,
									BHandler* target);
			BHandler*			_ApplyFilters(BList* list,
									BMessage* message, BHandler* target);

			void				_DispatchMessage(BMessage* message);

private:
			LooperLock*			fLock;
			MessageTarget*		fMessageTarget;
			BMessage*			fLastMessage;
			QList<BHandler*>	fHandlers;
			BList*				fCommonFilters;
			BHandler*			fPreferred;
};


class BLooper::MessageTarget : public QObject, public Referenceable {
public:
	explicit					MessageTarget(BLooper* looper);
								~MessageTarget();

			BLooper*			Looper() const
									{ return fLooper; }

protected:
	virtual	void				customEvent(QEvent* event);

private:
			void				LooperDeleted()
									{ fLooper = NULL; }

private:
			BLooper*			fLooper;
};


struct BLooper::Manager {
	Manager()
		:
		fMutex(),
		fLoopersByToken()
	{
	}

	static Manager* GetManager()
	{
		static Manager* manager = new Manager;
		return manager;
	}

	bool Lock()
	{
		fMutex.lock();
		return true;
	}

	void Unlock()
	{
		fMutex.unlock();
	}

	void RegisterLooperUnlocked(BLooper* looper)
	{
		QMutexLocker mutexLocker(&fMutex);
		fLoopersByToken.insert(looper->Token(), looper);
		fLoopers.insert(looper);
	}

	void UnregisterLooper(BLooper* looper)
	{
		fLoopersByToken.remove(looper->Token());
		fLoopers.remove(looper);
	}

	BLooper* LooperForToken(int32 token)
	{
		return fLoopersByToken.value(token, NULL);
	}

	bool IsLooperValid(const BLooper* looper) const
	{
		return fLoopers.contains(looper);
	}

private:
	QMutex					fMutex;
	QHash<int32, BLooper*>	fLoopersByToken;
	QSet<const BLooper*>	fLoopers;
};


#endif // PLATFORM_QT_B_LOOPER_H
