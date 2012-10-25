#include <Messenger.h>

#include <new>

#include <QCoreApplication>
#include <QEventLoop>

#include <MessageUtils.h>

#include "PlatformMessageEvent.h"


struct BMessenger::SynchronousReplyHandler : public QObject, public BHandler {
	SynchronousReplyHandler(BMessage* reply)
		:
		QObject(NULL),
		BHandler(),
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
	fHandlerProxy()
{
}


BMessenger::BMessenger(int32 handlerToken)
	:
	fHandlerProxy(BHandler::ProxyForToken(handlerToken))
{
}


BMessenger::BMessenger(const BHandler* handler, const BLooper* looper)
	:
	fHandlerProxy()
{
// TODO: looper is ignored ATM!
	if (handler != NULL)
		fHandlerProxy = handler->Proxy();
}


BMessenger::~BMessenger()
{
}


bool
BMessenger::IsValid() const
{
	return fHandlerProxy.toStrongRef() != NULL;
}


BHandler*
BMessenger::Target(BLooper** _looper) const
{
// TODO: That isn't thread safe!
	BHandler::ProxyPointer proxy = fHandlerProxy.toStrongRef();
	BHandler* handler = proxy != NULL ? proxy->Handler() : NULL;
	if (_looper != NULL) {
// TODO: Support looper!
		*_looper = NULL;
	}
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
	BHandler::ProxyPointer handlerProxy = fHandlerProxy.toStrongRef();
	if (handlerProxy == NULL)
		return B_BAD_HANDLER;

	PlatformMessageEvent* event = new(std::nothrow) PlatformMessageEvent(
		*message, replyTo.HandlerToken());
	if (event == NULL)
		return B_NO_MEMORY;

	QCoreApplication::postEvent(handlerProxy.data(), event);

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


int32
BMessenger::HandlerToken() const
{
	BHandler::ProxyPointer handlerProxy = fHandlerProxy.toStrongRef();
	if (handlerProxy == NULL)
		return B_NULL_TOKEN;
	return handlerProxy->Token();
}
