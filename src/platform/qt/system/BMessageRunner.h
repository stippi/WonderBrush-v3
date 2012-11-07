/*
 * Copyright 2001-2010, Haiku, Inc.
 * Distributed under the terms of the MIT License.
 */
#ifndef PLATFORM_QT_MESSAGE_RUNNER_H
#define PLATFORM_QT_MESSAGE_RUNNER_H


#include <Messenger.h>


class BMessageRunner {
public:
								BMessageRunner(BMessenger target,
									const BMessage* message, bigtime_t interval,
									int32 count = -1);
								BMessageRunner(BMessenger target,
									const BMessage& message, bigtime_t interval,
									int32 count = -1);
								BMessageRunner(BMessenger target,
									const BMessage* message, bigtime_t interval,
									int32 count, BMessenger replyTo);
								BMessageRunner(BMessenger target,
									const BMessage& message, bigtime_t interval,
									int32 count, BMessenger replyTo);
	virtual						~BMessageRunner();

			status_t			InitCheck() const;

			status_t			SetInterval(bigtime_t interval);
			status_t			SetCount(int32 count);
			status_t			GetInfo(bigtime_t* interval,
									int32* count) const;

	static	status_t			StartSending(BMessenger target,
									const BMessage* message, bigtime_t interval,
									int32 count);
	static	status_t			StartSending(BMessenger target,
									const BMessage* message, bigtime_t interval,
									int32 count, BMessenger replyTo);

private:
			struct Timer;

private:
								BMessageRunner(const BMessageRunner &);
								BMessageRunner &operator=(
									const BMessageRunner &);

	static	status_t			_RegisterRunner(BMessenger target,
									const BMessage* message, bigtime_t interval,
									int32 count, bool detach,
									BMessenger replyTo, Timer*& _timer);

			void				_InitData(BMessenger target,
									const BMessage* message, bigtime_t interval,
									int32 count, BMessenger replyTo);
			status_t			_SetParams(bool resetInterval,
									bigtime_t interval, bool resetCount,
									int32 count);

private:
			Timer*				fTimer;
			status_t			fInitStatus;
};


#endif	// PLATFORM_QT_MESSAGE_RUNNER_H
