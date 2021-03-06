/*
 * Copyright 2006-2007, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */

#include "Notifier.h"

#include <stdio.h>
#include <typeinfo>

#include <OS.h>

#include "Listener.h"

// constructor
Notifier::Notifier()
	: fListeners(2),
	  fSuspended(0),
	  fPendingNotifications(false)
{
}

// destructor
Notifier::~Notifier()
{
	if (fListeners.CountItems() > 0) {
		char message[256];
		Listener* o = (Listener*)fListeners.ItemAt(0);
		sprintf(message, "Notifier::~Notifier() - %" B_PRId32 " "
						 "listeners still watching, first: %s\n",
						 fListeners.CountItems(), typeid(*o).name());
		debugger(message);
	}
}

// AddListener
bool
Notifier::AddListener(Listener* listener)
{
	if (listener && !fListeners.HasItem((void*)listener)) {
		return fListeners.AddItem((void*)listener);
	}
	return false;
}

// RemoveListener
bool
Notifier::RemoveListener(Listener* listener)
{
	return fListeners.RemoveItem((void*)listener);
}

// CountListeners
int32
Notifier::CountListeners() const
{
	return fListeners.CountItems();
}

// ListenerAtFast
Listener*
Notifier::ListenerAtFast(int32 index) const
{
	return (Listener*)fListeners.ItemAtFast(index);
}

// #pragma mark -

// Notify
void
Notifier::Notify()
{
	if (!fSuspended) {
		NotifyListeners();
		fPendingNotifications = false;
	} else {
		fPendingNotifications = true;
	}
}

// SuspendNotifications
void
Notifier::SuspendNotifications(bool suspend)
{
	if (suspend)
		fSuspended++;
	else
		fSuspended--;

	if (fSuspended < 0) {
		fprintf(stderr, "Notifier::SuspendNotifications(false) - "
			"error: suspend level below zero: %" B_PRId32 "\n", fSuspended);
		debugger("");
		fSuspended = 0;
	}

	if (!fSuspended && fPendingNotifications)
		Notify();
}

// NotifyListeners
void
Notifier::NotifyListeners()
{
	BList listeners(fListeners);
	int32 count = listeners.CountItems();
	for (int32 i = 0; i < count; i++)
		((Listener*)listeners.ItemAtFast(i))->ObjectChanged(this);
}

