/*
 * Copyright 2006, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */

#include "BaseObject.h"

#include <Message.h>

#include <new>

#include "CommonPropertyIDs.h"
#include "Property.h"
#include "PropertyObject.h"

// constructor
BaseObject::BaseObject(const char* name)
	: Notifier(),
	  Referenceable(),
	  Selectable(),

	  fName(name)
{
}

// copy constructor
BaseObject::BaseObject(const BaseObject& other)
	: Notifier(),
	  Referenceable(),
	  Selectable(),

	  fName(other.fName)
{
}

// archive constructor
BaseObject::BaseObject(BMessage* archive)
	: Notifier(),
	  Referenceable(),
	  Selectable(),

	  fName()
{
	// NOTE: uses BaseObject version, not overridden
	Unarchive(archive);
}

// destructor
BaseObject::~BaseObject()
{
}

// SelectedChanged
void
BaseObject::SelectedChanged()
{
	// simply pass on the event for now
//	Notify();
}

// #pragma mark -

// Unarchive
status_t
BaseObject::Unarchive(const BMessage* archive)
{
	if (!archive)
		return B_BAD_VALUE;

	const char* name;
	status_t ret = archive->FindString("name", &name);

	if (ret == B_OK)
		fName = name;

	return ret;
}

// Archive
status_t
BaseObject::Archive(BMessage* into, bool deep) const
{
	if (!into)
		return B_BAD_VALUE;

	return into->AddString("name", fName.String());
}

// MakePropertyObject
PropertyObject*
BaseObject::MakePropertyObject() const
{
	PropertyObject* object = new(std::nothrow) PropertyObject();
	if (object != NULL) {
		object->AddProperty(new(std::nothrow) StringProperty(PROPERTY_NAME,
			fName.String()));
	}

	return object;
}

// SetToPropertyObject
bool
BaseObject::SetToPropertyObject(const PropertyObject* object)
{
	AutoNotificationSuspender _(this);

	BString name;
	if (object->GetValue(PROPERTY_NAME, name))
		SetName(name.String());

	return HasPendingNotifications();
}

// SetName
void
BaseObject::SetName(const char* name)
{
	if (fName == name)
		return;

	fName = name;
	Notify();
}
