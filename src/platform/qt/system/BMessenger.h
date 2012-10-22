#ifndef PLATFORM_QT_MESSENGER_H
#define PLATFORM_QT_MESSENGER_H


#include <Handler.h>
#include <SupportDefs.h>

#include <QWeakPointer>


class BMessage;


class BMessenger {
public:
								BMessenger();
								BMessenger(BHandler* handler);

			status_t			SendMessage(BMessage* message) const;

private:
			QWeakPointer<BHandlerProxy> fHandlerProxy;
};


#endif // PLATFORM_QT_MESSENGER_H
