/*
 * Copyright 2018, Stephan Aßmus <superstippi@gmx.de>.
 * Distributed under the terms of the MIT License.
 */
#include "FilterContrast.h"

#include <new>

#include "FilterContrastSnapshot.h"

// constructor
FilterContrast::FilterContrast()
	:
	Object(),
	fContrast(1.0),
	fCenter(128)
{
}

// constructor
FilterContrast::FilterContrast(const FilterContrast& other)
	:
	Object(other),
	fContrast(other.fContrast),
	fCenter(other.fCenter)
{
}

// constructor
FilterContrast::FilterContrast(float contrast, uint8 center)
	:
	Object(),
	fContrast(contrast),
	fCenter(center)
{
}

// destructor
FilterContrast::~FilterContrast()
{
}

// #pragma mark - BaseObject

// Clone
BaseObject*
FilterContrast::Clone(CloneContext& context) const
{
	return new(std::nothrow) FilterContrast(*this);
}


// DefaultName
const char*
FilterContrast::DefaultName() const
{
	return "Contrast";
}

// AddProperties
void
FilterContrast::AddProperties(PropertyObject* object, uint32 flags) const
{
	BaseObject::AddProperties(object, flags);

	object->AddProperty(new (std::nothrow) FloatProperty(
		PROPERTY_CONTRAST, fContrast, 0.0f, 10.0f));
	object->AddProperty(new (std::nothrow) IntProperty(
		PROPERTY_CENTER, fCenter, 0, 255));
}

// SetToPropertyObject
bool
FilterContrast::SetToPropertyObject(const PropertyObject* object, uint32 flags)
{
	AutoNotificationSuspender _(this);
	BaseObject::SetToPropertyObject(object, flags);

	// contrast
	SetContrast(object->Value(PROPERTY_CONTRAST, fContrast));
	// center
	SetCenter(object->Value(PROPERTY_CENTER, (int32)fCenter));

	return HasPendingNotifications();
}

// #pragma mark -

// Snapshot
ObjectSnapshot*
FilterContrast::Snapshot() const
{
	return new FilterContrastSnapshot(this);
}

// #pragma mark -

// IsRegularTransformable
bool
FilterContrast::IsRegularTransformable() const
{
	return false;
}

// SetContrast
void
FilterContrast::SetContrast(float saturation)
{
	if (fContrast == saturation)
		return;

	fContrast = saturation;

	UpdateChangeCounter();
	InvalidateParent();
	Notify();
}

// SetCenter
void
FilterContrast::SetCenter(uint8 center)
{
	if (fCenter == center)
		return;

	fCenter = center;

	UpdateChangeCounter();
	InvalidateParent();
	Notify();
}

