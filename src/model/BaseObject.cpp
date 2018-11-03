/*
 * Copyright 2006-2018, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */

#include "BaseObject.h"

#include <Message.h>

#include <new>

#include "CloneContext.h"

// constructor
BaseObject::BaseObject()
	: Notifier(),
	  Referenceable(),

	  fName()
{
}

// copy constructor
BaseObject::BaseObject(const BaseObject& other)
	: Notifier(),
	  Referenceable(),

	  fName(other.fName)
{
}

// archive constructor
BaseObject::BaseObject(const BMessage* archive)
	: Notifier(),
	  Referenceable(),

	  fName()
{
	// NOTE: uses BaseObject version, not overridden
	Unarchive(archive);
}

// destructor
BaseObject::~BaseObject()
{
}

// Clone
BaseObject*
BaseObject::Clone() const
{
	CloneContext context;
	return Clone(context);
}

// #pragma mark -

// Unarchive
status_t
BaseObject::Unarchive(const BMessage* archive)
{
	if (archive == NULL)
		return B_BAD_VALUE;

	const char* name;
	if (archive->FindString("name", &name) == B_OK)
		fName = name;

	return B_OK;
}

// Archive
status_t
BaseObject::Archive(BMessage* into, bool deep) const
{
	if (into == NULL)
		return B_BAD_VALUE;

	if (fName.Length() > 0)
		return into->AddString("name", fName.String());

	return B_OK;
}

// MakePropertyObject
PropertyObject*
BaseObject::MakePropertyObject() const
{
	PropertyObject* object = new(std::nothrow) PropertyObject();

	if (object != NULL)
		AddProperties(object);

	return object;
}

// AddProperties
void
BaseObject::AddProperties(PropertyObject* object, uint32 flags) const
{
	if ((flags & DONT_ADD_NAME) == 0) {
		object->AddProperty(new(std::nothrow) StringProperty(PROPERTY_NAME,
			fName.String()));
	}
}

// SetToPropertyObject
bool
BaseObject::SetToPropertyObject(const PropertyObject* object, uint32 flags)
{
	AutoNotificationSuspender _(this);

	if ((flags & DONT_ADD_NAME) == 0) {
		BString name;
		if (object->GetValue(PROPERTY_NAME, name))
			SetName(name.String());
	}

	return HasPendingNotifications();
}

// #pragma mark -

// SetName
void
BaseObject::SetName(const char* name)
{
	if (name == NULL || fName == name)
		return;

	fName = name;
	Notify();
}

// Name
const char*
BaseObject::Name() const
{
	if (fName.Length() > 0)
		return fName.String();
	return DefaultName();
}

// GivenName
const BString&
BaseObject::GivenName() const
{
	return fName;
}

// GetIcon
bool
BaseObject::GetIcon(const BBitmap* bitmap) const
{
	return false;
}

