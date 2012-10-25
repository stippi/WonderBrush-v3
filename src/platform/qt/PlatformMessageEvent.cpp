#include "PlatformMessageEvent.h"

#include <MessagePrivate.h>


PlatformMessageEvent::PlatformMessageEvent(const BMessage& message,
	int32 replyHandlerToken)
	:
	QEvent((QEvent::Type)EventType()),
	fMessage(message)
{
	BMessage::message_header* header
		= BMessage::Private(fMessage).GetMessageHeader();
	header->reply_port = 0;
	header->reply_target = replyHandlerToken;
	header->reply_team = 0;
}


/*static*/ int
PlatformMessageEvent::EventType()
{
	static int type = QEvent::registerEventType();
	return type;
}
