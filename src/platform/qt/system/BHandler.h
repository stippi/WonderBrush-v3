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
#include <String.h>

#include <QObject>
#include <QPointer>
#include <QSharedPointer>


class BHandler;
class BLooper;
class BMessage;


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

private:
			char*				fName;
			int32				fToken;
			BLooper*			fLooper;
			BHandler*			fNextHandler;
};


#endif // PLATFORM_QT_BHANDLER_H
