/*
 * Copyright 2001-2011, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Erik Jaesler (erik@cgsoftware.com)
 *		DarkWyrm (bpmagic@columbus.rr.com)
 *		Ingo Weinhold, bonefish@@users.sf.net
 *		Axel Dörfler, axeld@pinc-software.de
 */


#include "BLooper.h"

#include <new>

#include <List.h>
#include <Locker.h>
#include <MessageFilter.h>
#include <Messenger.h>

#include <MessagePrivate.h>

#include "AutoLocker.h"
#include "PlatformMessageEvent.h"


#define FILTER_LIST_BLOCK_SIZE	5


// #pragma mark - BLooper::MessageTarget


BLooper::MessageTarget::MessageTarget(BLooper* looper)
	:
	fLooper(looper)
{
}


BLooper::MessageTarget::~MessageTarget()
{
}


void
BLooper::MessageTarget::customEvent(QEvent* event)
{
	if (fLooper != NULL && event->type() == PlatformMessageEvent::EventType()) {
		fLooper->_DispatchMessage(
			&dynamic_cast<PlatformMessageEvent*>(event)->Message());
	}
}


// #pragma mark - BLooper::LooperLock


struct BLooper::LooperLock : Referenceable {
	LooperLock(const char* name)
		:
		fLock(name),
		fLooperAlive(true)
	{
	}

	bool Lock()
	{
		if (!fLock.Lock())
			return false;

		if (!fLooperAlive) {
			fLock.Unlock();
			return false;
		}

		return true;
	}

	status_t LockWithTimeout(bigtime_t timeout)
	{
		status_t error = fLock.LockWithTimeout(timeout);
		if (error != B_OK)
			return error;

		if (!fLooperAlive) {
			fLock.Unlock();
			return B_ERROR;
		}

		return B_OK;
	}

	void Unlock()
	{
		fLock.Unlock();
	}

	bool IsLocked() const
	{
		return fLock.IsLocked();
	}

private:
	BLocker	fLock;
	bool	fLooperAlive;
};


// #pragma mark - BLooper


BLooper::BLooper(const char* name, int32 priority, int32 /*portCapacity*/)
	:
	BHandler(name)
{
	_InitData(name, priority);
}


BLooper::~BLooper()
{
	Lock();

	Manager* manager = Manager::GetManager();
	AutoLocker<Manager> managerLocker(manager);

	RemoveHandler(this);

	// Remove all the "child" handlers
	int32 count = fHandlers.count();
	for (int32 i = 0; i < count; i++) {
		BHandler* handler = fHandlers.at(i);
		handler->SetNextHandler(NULL);
		handler->_SetLooper(NULL);
	}
	fHandlers.clear();

	manager->UnregisterLooper(this);

	if (fMessageTarget != NULL)
		fMessageTarget->RemoveReference();

	if (fLock != NULL) {
		fLock->Unlock();
		fLock->RemoveReference();
	}
}


status_t
BLooper::PostMessage(uint32 command)
{
	return BMessenger(NULL, this).SendMessage(command);
}


status_t
BLooper::PostMessage(BMessage* message)
{
	return BMessenger(NULL, this).SendMessage(message);
}


status_t
BLooper::PostMessage(uint32 command, BHandler* handler, BHandler* replyTo)
{
	return BMessenger(handler, this).SendMessage(command, replyTo);
}


status_t
BLooper::PostMessage(BMessage* message, BHandler* handler, BHandler* replyTo)
{
	return BMessenger(handler, this).SendMessage(message, replyTo);
}


void
BLooper::DispatchMessage(BMessage* message, BHandler* handler)
{
	switch (message->what) {
// TODO:...
#if 0
		case _QUIT_:
			// Can't call Quit() to do this, because of the slight chance
			// another thread with have us locked between now and then.
			fTerminating = true;

			// After returning from DispatchMessage(), the looper will be
			// deleted in _task0_()
			break;

		case B_QUIT_REQUESTED:
			if (handler == this) {
				_QuitRequested(message);
				break;
			}

			// fall through
#endif

		default:
			handler->MessageReceived(message);
			break;
	}
}


void
BLooper::AddHandler(BHandler* handler)
{
	if (handler == NULL)
		return;

	AssertLocked();

	if (handler->Looper() == NULL) {
		fHandlers.append(handler);
		handler->_SetLooper(this);
		if (handler != this)	// avoid a cycle
			handler->SetNextHandler(this);
	}
}


bool
BLooper::RemoveHandler(BHandler* handler)
{
	if (handler == NULL)
		return false;

	AssertLocked();

	if (handler->Looper() == this && fHandlers.removeOne(handler)) {
		if (handler == fPreferred)
			fPreferred = NULL;

		handler->SetNextHandler(NULL);
		handler->_SetLooper(NULL);
		return true;
	}

	return false;
}


int32
BLooper::CountHandlers() const
{
	AssertLocked();

	return fHandlers.count();
}


BHandler*
BLooper::HandlerAt(int32 index) const
{
	AssertLocked();

	return fHandlers.value(index, NULL);
}


int32
BLooper::IndexOf(BHandler* handler) const
{
	AssertLocked();

	return fHandlers.indexOf(handler);
}


BHandler*
BLooper::PreferredHandler() const
{
	return fPreferred;
}


void
BLooper::SetPreferredHandler(BHandler* handler)
{
	if (handler && handler->Looper() == this && IndexOf(handler) >= 0) {
		fPreferred = handler;
	} else {
		fPreferred = NULL;
	}
}


bool
BLooper::Lock()
{
	return _Lock(B_INFINITE_TIMEOUT) == B_OK;
}


void
BLooper::Unlock()
{
	fLock->Unlock();
}


bool
BLooper::IsLocked() const
{
	Manager* manager = Manager::GetManager();
	AutoLocker<Manager> managerLocker(manager);
	if (!manager->IsLooperValid(this))
		return false;

	return fLock != NULL && fLock->IsLocked();
}


status_t
BLooper::LockWithTimeout(bigtime_t timeout)
{
	return _Lock(timeout);
}


void
BLooper::AddCommonFilter(BMessageFilter* filter)
{
	if (!filter)
		return;

	AssertLocked();

	if (filter->Looper()) {
		debugger("A MessageFilter can only be used once.");
		return;
	}

	if (!fCommonFilters)
		fCommonFilters = new BList(FILTER_LIST_BLOCK_SIZE);

	filter->SetLooper(this);
	fCommonFilters->AddItem(filter);
}


bool
BLooper::RemoveCommonFilter(BMessageFilter* filter)
{
	AssertLocked();

	if (!fCommonFilters)
		return false;

	bool result = fCommonFilters->RemoveItem(filter);
	if (result)
		filter->SetLooper(NULL);

	return result;
}


void
BLooper::SetCommonFilterList(BList* filters)
{
	AssertLocked();

	BMessageFilter* filter;
	if (filters) {
		// Check for ownership issues - a filter can only have one owner
		for (int32 i = 0; i < filters->CountItems(); ++i) {
			filter = (BMessageFilter*)filters->ItemAt(i);
			if (filter->Looper()) {
				debugger("A MessageFilter can only be used once.");
				return;
			}
		}
	}

	if (fCommonFilters) {
		for (int32 i = 0; i < fCommonFilters->CountItems(); ++i) {
			delete (BMessageFilter*)fCommonFilters->ItemAt(i);
		}

		delete fCommonFilters;
		fCommonFilters = NULL;
	}

	// Per the BeBook, we take ownership of the list
	fCommonFilters = filters;
	if (fCommonFilters) {
		for (int32 i = 0; i < fCommonFilters->CountItems(); ++i) {
			filter = (BMessageFilter*)fCommonFilters->ItemAt(i);
			filter->SetLooper(this);
		}
	}
}


BList*
BLooper::CommonFilterList() const
{
	return fCommonFilters;
}


void
BLooper::ObjectConstructed(QObject* object)
{
	if (fMessageTarget != NULL)
		fMessageTarget->setParent(object);
}


void
BLooper::ObjectAboutToBeDestroyed(QObject* /*object*/)
{
	if (fMessageTarget != NULL)
		fMessageTarget->setParent(NULL);
}


void
BLooper::_InitData(const char* name, int32 priority)
{
	fLastMessage = NULL;
	fPreferred = NULL;
	fLock = new(std::nothrow) LooperLock(name);
	fMessageTarget = new(std::nothrow) MessageTarget(this);
	fCommonFilters = NULL;

#if 0
	fOwner = B_ERROR;
	fCachedStack = 0;
	fRunCalled = false;
	fDirectTarget = new (std::nothrow) BPrivate::BDirectMessageTarget();
	fThread = B_ERROR;
	fTerminating = false;
	fMsgPort = -1;
	fAtomicCount = 0;

	if (name == NULL)
		name = "anonymous looper";

#if DEBUG
	fLockSem = create_sem(1, name);
#else
	fLockSem = create_sem(0, name);
#endif

	if (portCapacity <= 0)
		portCapacity = B_LOOPER_PORT_DEFAULT_CAPACITY;

	fMsgPort = create_port(portCapacity, name);

	fInitPriority = priority;

	gLooperList.AddLooper(this);
		// this will also lock this looper
#endif

	Manager::GetManager()->RegisterLooperUnlocked(this);
	Lock();

	AddHandler(this);
}


status_t
BLooper::_Lock(bigtime_t timeout)
{
	// check whether we still exist
	Manager* manager = Manager::GetManager();
	AutoLocker<Manager> managerLocker(manager);
	if (!manager->IsLooperValid(this))
		return B_ERROR;

	// get a reference to the lock object
	if (fLock == NULL)
		return B_ERROR;
	Reference<LooperLock> lockReference(fLock);

	managerLocker.Unlock();

	return timeout == B_INFINITE_TIMEOUT
		? (fLock->Lock() ? B_OK : B_ERROR)
		: fLock->LockWithTimeout(timeout);
}


bool
BLooper::AssertLocked() const
{
	if (!IsLocked()) {
		debugger("looper must be locked before proceeding\n");
		return false;
	}

	return true;
}


BHandler*
BLooper::_TopLevelFilter(BMessage* message, BHandler* target)
{
	if (message == NULL)
		return target;

	// Apply the common filters first
	target = _ApplyFilters(CommonFilterList(), message, target);
	if (target) {
		if (target->Looper() != this) {
			debugger("Targeted handler does not belong to the looper.");
			target = NULL;
		} else {
			// Now apply handler-specific filters
//			target = _HandlerFilter(message, target);
		}
	}

	return target;
}


BHandler*
BLooper::_ApplyFilters(BList* list, BMessage* message, BHandler* target)
{
	// This is where the action is!
	// Check the parameters
	if (!list || !message)
		return target;

	// For each filter in the provided list
	BMessageFilter* filter = NULL;
	for (int32 i = 0; i < list->CountItems(); ++i) {
		filter = (BMessageFilter*)list->ItemAt(i);

		// Check command conditions
		if (filter->FiltersAnyCommand() || filter->Command() == message->what) {
			// Check delivery conditions
			message_delivery delivery = filter->MessageDelivery();
			bool dropped = message->WasDropped();
			if (delivery == B_ANY_DELIVERY
				|| (delivery == B_DROPPED_DELIVERY && dropped)
				|| (delivery == B_PROGRAMMED_DELIVERY && !dropped)) {
				// Check source conditions
				message_source source = filter->MessageSource();
				bool remote = false;
//				bool remote = message->IsSourceRemote();
				if (source == B_ANY_SOURCE
					|| (source == B_REMOTE_SOURCE && remote)
					|| (source == B_LOCAL_SOURCE && !remote)) {
					// Are we using an "external" function?
					filter_result result;
					filter_hook func = filter->FilterFunction();
					if (func)
						result = func(message, &target, filter);
					else
						result = filter->Filter(message, &target);

					// Is further processing allowed?
					if (result == B_SKIP_MESSAGE) {
						// No; time to bail out
						return NULL;
					}
				}
			}
		}
	}

	return target;
}


void
BLooper::_DispatchMessage(BMessage* message)
{
	Lock();

	fLastMessage = message;

	// Get the target handler
	BHandler *handler = NULL;
	BMessage::Private messagePrivate(fLastMessage);
	bool usePreferred = messagePrivate.UsePreferredTarget();

	if (usePreferred) {
		handler = fPreferred;
		if (handler == NULL)
			handler = this;
	} else {
		handler = BHandler::HandlerForToken(messagePrivate.GetTarget());

		// if this handler doesn't belong to us, we drop the message
		if (handler != NULL && handler->Looper() != this)
			handler = NULL;
	}

	// Is this a scripting message? (BMessage::HasSpecifiers())
#if 0
	if (handler != NULL && fLastMessage->HasSpecifiers()) {
		int32 index = 0;
		// Make sure the current specifier is kosher
		if (fLastMessage->GetCurrentSpecifier(&index) == B_OK)
			handler = resolve_specifier(handler, fLastMessage);
	}
#endif

	if (handler) {
		// Do filtering
		handler = _TopLevelFilter(fLastMessage, handler);
		if (handler && handler->Looper() == this)
			DispatchMessage(fLastMessage, handler);
	}

// TODO:...
#if 0
	if (fTerminating) {
		// we leave the looper locked when we quit
		return;
	}
#endif

	fLastMessage = NULL;

	// Unlock the looper
	Unlock();
}
