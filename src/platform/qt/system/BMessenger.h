#ifndef PLATFORM_QT_MESSENGER_H
#define PLATFORM_QT_MESSENGER_H


#include <SupportDefs.h>


class BMessage;


class BMessenger {
public:
// TODO:...
			status_t			SendMessage(BMessage* message) const;
};


#endif // PLATFORM_QT_MESSENGER_H
