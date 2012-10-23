/*
 * Copyright 2001-2005, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Erik Jaesler (erik@cgsoftware.com)
 */
#ifndef PLATFORM_QT_B_MESSAGE_FILTER_H
#define PLATFORM_QT_B_MESSAGE_FILTER_H


#include <Handler.h>


class BMessage;
class BMessageFilter;


// filter_hook Return Codes and Protocol ---------------------------------------
enum filter_result {
	B_SKIP_MESSAGE,
	B_DISPATCH_MESSAGE
};

typedef filter_result (*filter_hook)
	(BMessage* message, BHandler** target, BMessageFilter* filter);


// BMessageFilter invocation criteria ------------------------------------------
enum message_delivery {
	B_ANY_DELIVERY,
	B_DROPPED_DELIVERY,
	B_PROGRAMMED_DELIVERY
};

enum message_source {
	B_ANY_SOURCE,
	B_REMOTE_SOURCE,
	B_LOCAL_SOURCE
};


class BMessageFilter
{
public:
								BMessageFilter(uint32 what,
									filter_hook func = NULL);
								BMessageFilter(message_delivery delivery,
									message_source source,
									filter_hook func = NULL);
								BMessageFilter(message_delivery delivery,
									message_source source, uint32 what,
									filter_hook func = NULL);
								BMessageFilter(const BMessageFilter& filter);
								BMessageFilter(const BMessageFilter* filter);
	virtual						~BMessageFilter();

	// Hook function; ignored if filter_hook is non-NULL
	virtual	filter_result		Filter(BMessage* message, BHandler** _target);
};


#endif // PLATFORM_QT_B_MESSAGE_FILTER_H
