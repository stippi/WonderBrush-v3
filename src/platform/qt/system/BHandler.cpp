#include "BHandler.h"

#include <string.h>

#include <new>

#include <MessageUtils.h>

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
		handler->Proxy()->HandlerDeleted();
	}

	int32 GetHandlerToken(const BHandlerProxy* proxy)
	{
		QMutexLocker mutexLocker(&fMutex);
		if (proxy->Handler() == NULL)
			return B_NULL_TOKEN;
		return proxy->Handler()->Token();
	}

	BHandler::ProxyPointer ProxyForToken(int32 token)
	{
		QMutexLocker mutexLocker(&fMutex);
		BHandler* handler = fHandlers.value(token, NULL);
		return handler != NULL ? handler->Proxy() : BHandler::ProxyPointer();
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


// #pragma mark - BHandlerProxy


BHandlerProxy::BHandlerProxy(BHandler* handler)
	:
	fHandler(handler)
{
}


BHandlerProxy::~BHandlerProxy()
{
}


int32
BHandlerProxy::Token() const
{
	return Manager::GetManager()->GetHandlerToken(this);
}


void
BHandlerProxy::customEvent(QEvent* event)
{
	if (fHandler != NULL && event->type()
			== PlatformMessageEvent::EventType()) {
		fHandler->MessageReceived(
			&dynamic_cast<PlatformMessageEvent*>(event)->Message());
	}
}


// #pragma mark - BHandler


BHandler::BHandler(BMessage* archive)
	:
	fName(NULL),
	fProxy(new(std::nothrow) BHandlerProxy(this)),
	fToken(B_NULL_TOKEN)
{
	debugger("BHandler unarchiving constructor unsupported");
}


BHandler::BHandler(const char* name)
	:
	fName(NULL),
	fProxy(new(std::nothrow) BHandlerProxy(this)),
	fToken(B_NULL_TOKEN)
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


/*static*/ BHandler::ProxyPointer
BHandler::ProxyForToken(int32 token)
{
	return Manager::GetManager()->ProxyForToken(token);
}


void
BHandler::ObjectConstructed(QObject* object)
{
	fProxy->setParent(object);
}


void
BHandler::ObjectAboutToBeDestroyed(QObject* /*object*/)
{
	fProxy->setParent(NULL);
}
