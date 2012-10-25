#ifndef PLATFORM_QT_MESSAGE_EVENT_H
#define PLATFORM_QT_MESSAGE_EVENT_H


#include <Message.h>

#include <QEvent>


class PlatformMessageEvent : public QEvent
{
public:
	explicit					PlatformMessageEvent(const BMessage& message,
									int32 replyHandlerToken);

			const BMessage&		Message() const
									{ return fMessage; }
			BMessage&			Message()
									{ return fMessage; }

	static	int					EventType();

private:
			BMessage			fMessage;
			int32				fReplyHandlerToken;
};


#endif // PLATFORM_QT_MESSAGE_EVENT_H
