#include "PlatformMessageEvent.h"

PlatformMessageEvent::PlatformMessageEvent(const BMessage& message)
	:
	QEvent((QEvent::Type)EventType()),
	fMessage(message)
{
}


/*static*/ int
PlatformMessageEvent::EventType()
{
	static int type = QEvent::registerEventType();
	return type;
}
