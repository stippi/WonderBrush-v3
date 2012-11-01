/*
 * Copyright 2001-2007, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Erik Jaesler (erik@cgsoftware.com)
 *		Axel Dörfler, axeld@pinc-software.de
 */


#include "BHandler.h"

#include <stdio.h>
#include <string.h>

#include <map>
#include <new>
#include <vector>

#include <MessageUtils.h>
#include <Looper.h>

#include <QHash>
#include <QMutex>

#include "PlatformMessageEvent.h"


using std::map;
using std::vector;


static const uint32 kMsgStartObserving = '_OBS';
static const uint32 kMsgStopObserving = '_OBP';
static const char* kObserveTarget = "be:observe_target";


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


namespace BPrivate {

class ObserverList {
	public:
		ObserverList();
		~ObserverList();

		status_t SendNotices(uint32 what, const BMessage* notice);
		status_t Add(const BHandler* handler, uint32 what);
		status_t Add(const BMessenger& messenger, uint32 what);
		status_t Remove(const BHandler* handler, uint32 what);
		status_t Remove(const BMessenger& messenger, uint32 what);
		bool IsEmpty();

	private:
		typedef map<uint32, vector<const BHandler *> > HandlerObserverMap;
		typedef map<uint32, vector<BMessenger> > MessengerObserverMap;

		void _ValidateHandlers(uint32 what);
		void _SendNotices(uint32 what, BMessage* notice);

		HandlerObserverMap		fHandlerMap;
		MessengerObserverMap	fMessengerMap;
};

}	// namespace BPrivate

using namespace BPrivate;


// #pragma mark - BHandler


BHandler::BHandler(BMessage* archive)
	:
	fName(NULL),
	fToken(B_NULL_TOKEN),
	fLooper(NULL),
	fNextHandler(NULL),
	fObserverList(NULL)
{
	debugger("BHandler unarchiving constructor unsupported");
}


BHandler::BHandler(const char* name)
	:
	fName(NULL),
	fToken(B_NULL_TOKEN),
	fLooper(NULL),
	fNextHandler(NULL),
	fObserverList(NULL)
{
	SetName(name);

	Manager::GetManager()->RegisterHandler(this);
}


BHandler::~BHandler()
{
	// remove all observers (the observer list manages itself)
	delete fObserverList;

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
	BMessage reply(B_REPLY);

	switch (message->what) {
		case kMsgStartObserving:
		case kMsgStopObserving:
		{
			BMessenger target;
			uint32 what;
			if (message->FindMessenger(kObserveTarget, &target) != B_OK
				|| message->FindInt32(B_OBSERVE_WHAT_CHANGE, (int32*)&what) != B_OK)
				break;

			ObserverList* list = _ObserverList();
			if (list != NULL) {
				if (message->what == kMsgStartObserving)
					list->Add(target, what);
				else
					list->Remove(target, what);
			}
			break;
		}

#if 0
		case B_GET_PROPERTY:
		{
			int32 cur;
			BMessage specifier;
			int32 form;
			const char *prop;

			status_t err = message->GetCurrentSpecifier(&cur, &specifier,
				&form, &prop);
			if (err != B_OK && err != B_BAD_SCRIPT_SYNTAX)
				break;
			bool known = false;
			// B_BAD_SCRIPT_SYNTAX defaults to the Messenger property
			if (err == B_BAD_SCRIPT_SYNTAX || cur < 0
				|| (strcmp(prop, "Messenger") == 0)) {
				err = reply.AddMessenger("result", this);
				known = true;
			} else if (strcmp(prop, "Suites") == 0) {
				err = GetSupportedSuites(&reply);
				known = true;
			} else if (strcmp(prop, "InternalName") == 0) {
				err = reply.AddString("result", Name());
				known = true;
			}

			if (known) {
				reply.AddInt32("error", B_OK);
				message->SendReply(&reply);
				return;
			}
			// let's try next handler
			break;
		}

		case B_GET_SUPPORTED_SUITES:
		{
			reply.AddInt32("error", GetSupportedSuites(&reply));
			message->SendReply(&reply);
			return;
		}
#endif
	}

	// ToDo: there is some more work needed here (someone in the know should fill in)!

// TODO: Would calling the next handler work well with views?
//	if (fNextHandler) {
if (false) {
#if 0
		// we need to apply the next handler's filters here, too
		BHandler* target = Looper()->_HandlerFilter(message, fNextHandler);
		if (target != NULL && target != this) {
			// TODO: we also need to make sure that "target" is not before
			//	us in the handler chain - at least in case it wasn't before
			//	the handler actually targeted with this message - this could
			//	get ugly, though.
			target->MessageReceived(message);
		}
#endif
	} else if (message->what != B_MESSAGE_NOT_UNDERSTOOD
		&& (message->WasDropped() || message->HasSpecifiers())) {
		printf("BHandler %s: MessageReceived() couldn't understand the message:\n", Name());
		message->PrintToStream();
		message->SendReply(B_MESSAGE_NOT_UNDERSTOOD);
	}
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


status_t
BHandler::StartWatching(BMessenger target, uint32 what)
{
	BMessage message(kMsgStartObserving);
	message.AddMessenger(kObserveTarget, this);
	message.AddInt32(B_OBSERVE_WHAT_CHANGE, what);

	return target.SendMessage(&message);
}


status_t
BHandler::StartWatchingAll(BMessenger target)
{
	return StartWatching(target, B_OBSERVER_OBSERVE_ALL);
}


status_t
BHandler::StopWatching(BMessenger target, uint32 what)
{
	BMessage message(kMsgStopObserving);
	message.AddMessenger(kObserveTarget, this);
	message.AddInt32(B_OBSERVE_WHAT_CHANGE, what);

	return target.SendMessage(&message);
}


status_t
BHandler::StopWatchingAll(BMessenger target)
{
	return StopWatching(target, B_OBSERVER_OBSERVE_ALL);
}


status_t
BHandler::StartWatching(BHandler* handler, uint32 what)
{
	ObserverList* list = _ObserverList();
	if (list == NULL)
		return B_NO_MEMORY;

	return list->Add(handler, what);
}


status_t
BHandler::StartWatchingAll(BHandler* handler)
{
	return StartWatching(handler, B_OBSERVER_OBSERVE_ALL);
}


status_t
BHandler::StopWatching(BHandler* handler, uint32 what)
{
	ObserverList* list = _ObserverList();
	if (list == NULL)
		return B_NO_MEMORY;

	return list->Remove(handler, what);
}


status_t
BHandler::StopWatchingAll(BHandler *handler)
{
	return StopWatching(handler, B_OBSERVER_OBSERVE_ALL);
}


void
BHandler::SendNotices(uint32 what, const BMessage* notice)
{
	if (fObserverList != NULL)
		fObserverList->SendNotices(what, notice);
}


bool
BHandler::IsWatched() const
{
	return fObserverList && !fObserverList->IsEmpty();
}


BHandler*
BHandler::HandlerForToken(int32 token)
{
// TODO:...
	return Manager::GetManager()->HandlerForToken(token);
}


ObserverList*
BHandler::_ObserverList()
{
	if (fObserverList == NULL)
		fObserverList = new (std::nothrow) BPrivate::ObserverList();

	return fObserverList;
}


//	#pragma mark -


ObserverList::ObserverList()
{
}


ObserverList::~ObserverList()
{
}


void
ObserverList::_ValidateHandlers(uint32 what)
{
	vector<const BHandler *>& handlers = fHandlerMap[what];
	vector<const BHandler *>::iterator iterator = handlers.begin();

	while (iterator != handlers.end()) {
		BMessenger target(*iterator);
		if (!target.IsValid()) {
			iterator++;
			continue;
		}

		Add(target, what);
		iterator = handlers.erase(iterator);
	}
}


void
ObserverList::_SendNotices(uint32 what, BMessage* message)
{
	// first iterate over the list of handlers and try to make valid messengers out of them
	_ValidateHandlers(what);

	// now send it to all messengers we know
	vector<BMessenger>& messengers = fMessengerMap[what];
	vector<BMessenger>::iterator iterator = messengers.begin();

	while (iterator != messengers.end()) {
		if (!(*iterator).IsValid()) {
			iterator = messengers.erase(iterator);
			continue;
		}

		(*iterator).SendMessage(message);
		iterator++;
	}
}


status_t
ObserverList::SendNotices(uint32 what, const BMessage* message)
{
	BMessage *copy = NULL;
	if (message) {
		copy = new BMessage(*message);
		copy->what = B_OBSERVER_NOTICE_CHANGE;
		copy->AddInt32(B_OBSERVE_ORIGINAL_WHAT, message->what);
	} else
		copy = new BMessage(B_OBSERVER_NOTICE_CHANGE);

	copy->AddInt32(B_OBSERVE_WHAT_CHANGE, what);

	_SendNotices(what, copy);
	_SendNotices(B_OBSERVER_OBSERVE_ALL, copy);

	delete copy;
	return B_OK;
}


status_t
ObserverList::Add(const BHandler *handler, uint32 what)
{
	if (handler == NULL)
		return B_BAD_HANDLER;

	// if this handler already represents a valid target, add its messenger
	BMessenger target(handler);
	if (target.IsValid())
		return Add(target, what);

	vector<const BHandler*> &handlers = fHandlerMap[what];

	vector<const BHandler*>::iterator iter;
	iter = find(handlers.begin(), handlers.end(), handler);
	if (iter != handlers.end()) {
		// TODO: do we want to have a reference count for this?
		return B_OK;
	}

	handlers.push_back(handler);
	return B_OK;
}


status_t
ObserverList::Add(const BMessenger &messenger, uint32 what)
{
	vector<BMessenger> &messengers = fMessengerMap[what];

	vector<BMessenger>::iterator iter;
	iter = find(messengers.begin(), messengers.end(), messenger);
	if (iter != messengers.end()) {
		// TODO: do we want to have a reference count for this?
		return B_OK;
	}

	messengers.push_back(messenger);
	return B_OK;
}


status_t
ObserverList::Remove(const BHandler *handler, uint32 what)
{
	if (handler == NULL)
		return B_BAD_HANDLER;

	// look into the list of messengers
	BMessenger target(handler);
	if (target.IsValid() && Remove(target, what) == B_OK)
		return B_OK;

	vector<const BHandler*> &handlers = fHandlerMap[what];

	vector<const BHandler*>::iterator iterator = find(handlers.begin(),
		handlers.end(), handler);
	if (iterator != handlers.end()) {
		handlers.erase(iterator);
		if (handlers.empty())
			fHandlerMap.erase(what);

		return B_OK;
	}

	return B_BAD_HANDLER;
}


status_t
ObserverList::Remove(const BMessenger &messenger, uint32 what)
{
	vector<BMessenger> &messengers = fMessengerMap[what];

	vector<BMessenger>::iterator iterator = find(messengers.begin(),
		messengers.end(), messenger);
	if (iterator != messengers.end()) {
		messengers.erase(iterator);
		if (messengers.empty())
			fMessengerMap.erase(what);

		return B_OK;
	}

	return B_BAD_HANDLER;
}


bool
ObserverList::IsEmpty()
{
	return fHandlerMap.empty() && fMessengerMap.empty();
}
