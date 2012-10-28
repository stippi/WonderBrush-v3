/*
 * Copyright 2001-2007, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Erik Jaesler (erik@cgsoftware.com)
 *		Axel Dörfler, axeld@pinc-software.de
 */


#include "BHandler.h"

#include <string.h>

#include <new>

#include <MessageUtils.h>
#include <Looper.h>

#include <QHash>
#include <QMutex>

#include "PlatformMessageEvent.h"


// #pragma mark - Manager


namespace {


struct Manager {
	Manager()
		:
		fMutex(),
		fHandlers(),
		fNextToken(0)
	{
	}

	static Manager* GetManager()
	{
		static Manager* manager = new Manager;
		return manager;
	}

	void RegisterHandler(BHandler* handler)
	{
		QMutexLocker mutexLocker(&fMutex);
		int32 token = _NextToken();
		handler->SetToken(token);
		fHandlers.insert(token, handler);
	}

	void UnregisterHandler(BHandler* handler)
	{
		QMutexLocker mutexLocker(&fMutex);
		fHandlers.remove(handler->Token());
		handler->SetToken(B_NULL_TOKEN);
	}

	BHandler* HandlerForToken(int32 token)
	{
		QMutexLocker mutexLocker(&fMutex);
		return fHandlers.value(token, NULL);
	}

private:
	// Caller must hold fMutex.
	int32 _NextToken()

	{
		for (;;) {
			if (fNextToken < 0)
				fNextToken = 0;
			int32 token = fNextToken++;

			if (!fHandlers.contains(token))
				return token;
		}
	}

private:
	QMutex					fMutex;
	QHash<int32, BHandler*>	fHandlers;
	int32					fNextToken;
};


}	// unnamed namespace


// #pragma mark - BHandler


BHandler::BHandler(BMessage* archive)
	:
	fName(NULL),
	fToken(B_NULL_TOKEN),
	fLooper(NULL),
	fNextHandler(NULL)
{
	debugger("BHandler unarchiving constructor unsupported");
}


BHandler::BHandler(const char* name)
	:
	fName(NULL),
	fToken(B_NULL_TOKEN),
	fLooper(NULL),
	fNextHandler(NULL)
{
	SetName(name);

	Manager::GetManager()->RegisterHandler(this);
}


BHandler::~BHandler()
{
	Manager::GetManager()->UnregisterHandler(this);
	free(fName);
}


void
BHandler::SetName(const char* name)
{
	free(fName);
	fName = name != NULL ? strdup(name) : NULL;
}


void
BHandler::MessageReceived(BMessage* message)
{
}


void
BHandler::SetNextHandler(BHandler *handler)
{
	if (!fLooper) {
		debugger("handler must belong to looper before setting NextHandler");
		return;
	}

	if (!fLooper->IsLocked()) {
		debugger(
			"The handler's looper must be locked before setting NextHandler");
		return;
	}

	if (handler && fLooper != handler->Looper()) {
		debugger("The handler and its NextHandler must have the same looper");
		return;
	}

	fNextHandler = handler;
}


BHandler *
BHandler::NextHandler() const
{
	return fNextHandler;
}


void
BHandler::SendNotices(uint32 what, const BMessage* notice)
{
// TODO:...
}


bool
BHandler::IsWatched() const
{
// TODO:...
	return false;
}


BHandler*
BHandler::HandlerForToken(int32 token)
{
// TODO:...
	return Manager::GetManager()->HandlerForToken(token);
}
