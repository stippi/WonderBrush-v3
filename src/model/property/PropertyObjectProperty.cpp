/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "PropertyObjectProperty.h"

#include <new>
#include <stdio.h>

#include <Message.h>

// constructor
PropertyObjectProperty::PropertyObjectProperty(uint32 identifier)
	: Property(identifier)
{
}

// archive constructor
PropertyObjectProperty::PropertyObjectProperty(
		const PropertyObjectProperty& other)
	: Property(other)
	, fValue(other.fValue)
{
}

// archive constructor
PropertyObjectProperty::PropertyObjectProperty(BMessage* archive)
	: Property(archive)
{
	BMessage archivedObject;
	if (archive->FindMessage("value", &archivedObject) == B_OK)
		fValue.Unarchive(&archivedObject);
}

// destrucor
PropertyObjectProperty::~PropertyObjectProperty()
{
}

// Archive
status_t
PropertyObjectProperty::Archive(BMessage* into, bool deep) const
{
	status_t status = Property::Archive(into, deep);

	if (status == B_OK) {
		BMessage archivedObject;
		if (fValue.Archive(&archivedObject) == B_OK)
			status = into->AddMessage("value", &archivedObject);
	}

	// finish off
	if (status == B_OK)
		status = into->AddString("class", "PropertyObjectProperty");

	return status;
}

// Instantiate
BArchivable*
PropertyObjectProperty::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "PropertyObjectProperty"))
		return new(std::nothrow) PropertyObjectProperty(archive);
	return NULL;
}

// #pragma mark -

// Clone
Property*
PropertyObjectProperty::Clone() const
{
	return new(std::nothrow) PropertyObjectProperty(*this);
}

// SetValue
bool
PropertyObjectProperty::SetValue(const char* str)
{
	return false;
}

// SetValue
bool
PropertyObjectProperty::SetValue(const Property* other)
{
	const PropertyObjectProperty* p
		= dynamic_cast<const PropertyObjectProperty*>(other);
	if (p != NULL) {
		fValue = p->fValue;
		return true;
	}
	return false;
}

// GetValue
void
PropertyObjectProperty::GetValue(BString& string)
{
	string << "dummy";
}

// InterpolateTo
bool
PropertyObjectProperty::InterpolateTo(const Property* other, float scale)
{
	return false;
}

