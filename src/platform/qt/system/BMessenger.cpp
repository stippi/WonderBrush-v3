#include <Messenger.h>

#include <new>

#include <QCoreApplication>

#include "PlatformMessageEvent.h"


BMessenger::BMessenger()
	:
	fHandlerProxy()
{
}


BMessenger::BMessenger(BHandler* handler)
	:
	fHandlerProxy()
{
	if (handler != NULL)
		fHandlerProxy = handler->Proxy();
}


status_t
BMessenger::SendMessage(BMessage* message) const
{
	BHandler::ProxyPointer handlerProxy = fHandlerProxy.toStrongRef();
	if (handlerProxy == NULL)
		return B_BAD_HANDLER;


	PlatformMessageEvent* event
		= new(std::nothrow) PlatformMessageEvent(*message);
	if (event == NULL)
		return B_NO_MEMORY;

	QCoreApplication::postEvent(handlerProxy.data(), event);

	return B_OK;
}
