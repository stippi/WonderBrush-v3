#ifndef PLATFORM_QT_MESSENGER_H
#define PLATFORM_QT_MESSENGER_H


#include <Message.h>
#include <SupportDefs.h>

#include <QWeakPointer>


class BHandler;
class BLooper;


class BMessenger {
public:
								BMessenger();
								BMessenger(int32 handlerToken);
									// conceptually package private
								BMessenger(const BHandler* handler,
									const BLooper* looper = NULL,
									status_t* _result = NULL);
								~BMessenger();

			bool				IsValid() const;

			bool				IsTargetLocal() const
									{ return true; }
			BHandler*			Target(BLooper** _looper) const;

			status_t			SendMessage(uint32 command,
									BHandler* replyTo = NULL) const;
			status_t			SendMessage(BMessage* message,
									BHandler *replyTo = NULL,
									bigtime_t timeout = B_INFINITE_TIMEOUT)
									const;
			status_t			SendMessage(BMessage* message,
									BMessenger replyTo,
									bigtime_t timeout = B_INFINITE_TIMEOUT)
									const;
			status_t			SendMessage(uint32 command,
									BMessage* reply) const;
			status_t			SendMessage(BMessage *message, BMessage *reply,
									bigtime_t deliveryTimeout
										= B_INFINITE_TIMEOUT,
									bigtime_t replyTimeout = B_INFINITE_TIMEOUT)
										const;

			int32				LooperToken() const
									{ return fLooperToken; }
			int32				HandlerToken() const
									{ return fHandlerToken; }

			bool				operator==(const BMessenger& other) const;
			bool				operator!=(const BMessenger& other) const
									{ return !(*this == other); }

private:
			struct SynchronousReplyHandler;

private:
			int32				fLooperToken;
			int32				fHandlerToken;
};


#endif // PLATFORM_QT_MESSENGER_H
