/*
 * Copyright 2001-2007, Haiku Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Erik Jaesler (erik@cgsoftware.com)
 */
#ifndef PLATFORM_QT_BHANDLER_H
#define PLATFORM_QT_BHANDLER_H


#include <Archivable.h>
#include <Messenger.h>
#include <String.h>

#include <QObject>
#include <QPointer>
#include <QSharedPointer>


#define B_OBSERVE_WHAT_CHANGE "be:observe_change_what"
#define B_OBSERVE_ORIGINAL_WHAT "be:observe_orig_what"
const uint32 B_OBSERVER_OBSERVE_ALL = 0xffffffff;


class BHandler;
class BLooper;
class BMessage;

namespace BPrivate {
	class ObserverList;
}


class BHandler : public BArchivable
{
public:
								BHandler(BMessage* archive);
								BHandler(const char* name = NULL);
	virtual						~BHandler();

			const char*			Name() const
									{ return fName; }
			void				SetName(const char* name);

	virtual	void				MessageReceived(BMessage* message);

			BLooper*			Looper() const
									{ return fLooper; }
	virtual	void				SetNextHandler(BHandler* handler);
			BHandler*			NextHandler() const;

	// Observer calls, inter-looper and inter-team
			status_t			StartWatching(BMessenger target, uint32 what);
			status_t			StartWatchingAll(BMessenger target);
			status_t			StopWatching(BMessenger target, uint32 what);
			status_t			StopWatchingAll(BMessenger target);

	// Observer calls for observing targets in the local team
			status_t			StartWatching(BHandler* observer, uint32 what);
			status_t			StartWatchingAll(BHandler* observer);
			status_t			StopWatching(BHandler* observer, uint32 what);
			status_t			StopWatchingAll(BHandler* observer);


	virtual	void				SendNotices(uint32 what,
									const BMessage* notice = NULL);
			bool				IsWatched() const;

			int32				Token() const
									{ return fToken; }
	static	BHandler*			HandlerForToken(int32 token);

			// conceptually private
			void				SetToken(int32 token)
									{ fToken = token; }

private:
			friend class BLooper;

private:
			void				_SetLooper(BLooper* looper)
									{ fLooper = looper; }

			BPrivate::ObserverList* _ObserverList();

private:
			char*				fName;
			int32				fToken;
			BLooper*			fLooper;
			BHandler*			fNextHandler;
			BPrivate::ObserverList*	fObserverList;
};


#endif // PLATFORM_QT_BHANDLER_H
