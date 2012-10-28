/*
 * Copyright 2001-2011, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Ingo Weinhold (bonefish@users.sf.net)
 */

#include <Messenger.h>

#include <new>

#include <Looper.h>
#include <MessageUtils.h>

#include <QCoreApplication>
#include <QEventLoop>

#include "AutoLocker.h"
#include "PlatformMessageEvent.h"


struct BMessenger::SynchronousReplyHandler : public QObject, public BLooper {
	SynchronousReplyHandler(BMessage* reply)
		:
		QObject(NULL),
		BLooper(),
		fEventLoop(),
		fReply(reply),
		fTimer(0)
	{
	}

	bool WaitForReply(bigtime_t timeout)
	{
		if (timeout <= 0)
			return false;

		fAbsoluteTimeout = timeout == B_INFINITE_TIMEOUT
			? timeout : system_time() + timeout;
		if (!_ScheduleTimer())
			return false;

		fEventLoop.exec();

		if (fTimer != 0) {
			killTimer(fTimer);
			fTimer = 0;
		}

		return fReply == NULL;
	}

	virtual void MessageReceived(BMessage* message)
	{
		if (fReply != NULL) {
			*fReply = *message;
			fReply = NULL;
			fEventLoop.exit();
		}
	}

protected:
	virtual void timerEvent(QTimerEvent* event)
	{
		if (!_ScheduleTimer())
			fEventLoop.exit();
	}

private:
	bool _ScheduleTimer()
	{
		if (fTimer != 0) {
			killTimer(fTimer);
			fTimer = 0;
		}

		bigtime_t timeout = fAbsoluteTimeout - system_time();
		if (timeout <= 0)
			return false;

		fTimer = startTimer(timeout);
		return fTimer != 0;
	}

private:
	QEventLoop		fEventLoop;
	BMessage*		fReply;
	bigtime_t		fAbsoluteTimeout;
	int				fTimer;
};


BMessenger::BMessenger()
	:
	fLooperToken(B_NULL_TOKEN),
	fHandlerToken(B_NULL_TOKEN)
{
}


BMessenger::BMessenger(int32 handlerToken)
	:
	fLooperToken(B_NULL_TOKEN),
	fHandlerToken(B_NULL_TOKEN)
{
}


BMessenger::BMessenger(const BHandler* handler, const BLooper* looper,
	status_t* _result)
	:
	fLooperToken(B_NULL_TOKEN),
	fHandlerToken(B_NULL_TOKEN)
{
	status_t error = (handler || looper ? B_OK : B_BAD_VALUE);
	if (error == B_OK) {
		if (handler) {
			// BHandler is given, check/retrieve the looper.
			if (looper) {
				if (handler->Looper() != looper)
					error = B_MISMATCHED_VALUES;
			} else {
				looper = handler->Looper();
				if (looper == NULL)
					error = B_MISMATCHED_VALUES;
			}
		}

		// set port, token,...
		if (error == B_OK) {
			BLooper::Manager* looperManager = BLooper::Manager::GetManager();
			AutoLocker<BLooper::Manager> looperManagerLocker(looperManager);
			if (looperManager->IsLooperValid(looper)) {
				fLooperToken = looper->Token() ;
				fHandlerToken = handler != NULL
					? handler->Token() : B_PREFERRED_TOKEN;
			} else
				error = B_BAD_VALUE;
		}
	}
	if (_result)
		*_result = error;
}


BMessenger::~BMessenger()
{
}


bool
BMessenger::IsValid() const
{
	if (fHandlerToken == B_NULL_TOKEN)
		return false;

	BLooper::Manager* looperManager = BLooper::Manager::GetManager();
	AutoLocker<BLooper::Manager> looperManagerLocker(looperManager);
	return looperManager->LooperForToken(fLooperToken) != NULL;
}


BHandler*
BMessenger::Target(BLooper** _looper) const
{
	BHandler* handler = NULL;
	if (fHandlerToken > B_NULL_TOKEN || fHandlerToken == B_PREFERRED_TOKEN) {
		handler = BHandler::HandlerForToken(fHandlerToken);
		if (_looper != NULL) {
			BLooper::Manager* looperManager = BLooper::Manager::GetManager();
			AutoLocker<BLooper::Manager> looperManagerLocker(looperManager);
			*_looper = looperManager->LooperForToken(fLooperToken);
		}
	} else if (_looper)
		*_looper = NULL;

	return handler;
}


status_t
BMessenger::SendMessage(uint32 command, BHandler* replyTo) const
{
	BMessage message(command);
	return SendMessage(&message, replyTo);
}


status_t
BMessenger::SendMessage(BMessage* message, BHandler *replyTo,
	bigtime_t timeout) const
{
	return SendMessage(message, BMessenger(replyTo), timeout);
}


status_t
BMessenger::SendMessage(BMessage* message, BMessenger replyTo,
	bigtime_t /*timeout*/) const
{
	// get the looper's message target
	BLooper::Manager* looperManager = BLooper::Manager::GetManager();
	AutoLocker<BLooper::Manager> looperManagerLocker(looperManager);
	BLooper* looper = looperManager->LooperForToken(fLooperToken);
	if (looper == NULL)
		return B_BAD_HANDLER;

	BLooper::MessageTarget* messageTarget = looper->GetMessageTarget();
	if (messageTarget == NULL)
		return B_BAD_HANDLER;
	Reference<BLooper::MessageTarget> messageTargetReference(messageTarget);

	looperManagerLocker.Unlock();

	// send the message via a Qt event
	PlatformMessageEvent* event = new(std::nothrow) PlatformMessageEvent(
		*message, fHandlerToken, replyTo.fLooperToken, replyTo.fHandlerToken);
	if (event == NULL)
		return B_NO_MEMORY;

	QCoreApplication::postEvent(messageTarget, event);

	return B_OK;
}


status_t
BMessenger::SendMessage(uint32 command, BMessage* reply) const
{
	BMessage message(command);
	return SendMessage(&message, reply);
}


status_t
BMessenger::SendMessage(BMessage* message, BMessage* reply,
	bigtime_t deliveryTimeout, bigtime_t replyTimeout) const
{
	SynchronousReplyHandler replyHandler(reply);
	status_t error = SendMessage(message, BMessenger(&replyHandler),
		deliveryTimeout);
	if (error != B_OK)
		return error;

	return replyHandler.WaitForReply(replyTimeout) ? B_OK : B_TIMED_OUT;
}
