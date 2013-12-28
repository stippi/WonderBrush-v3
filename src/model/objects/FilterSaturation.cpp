/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * Distributed under the terms of the MIT License.
 */
#include "FilterSaturation.h"

#include <new>

#include "FilterSaturationSnapshot.h"

// constructor
FilterSaturation::FilterSaturation()
	:
	Object(),
	fSaturation(1.0)
{
}

// constructor
FilterSaturation::FilterSaturation(const FilterSaturation& other)
	:
	Object(other),
	fSaturation(other.fSaturation)
{
}

// constructor
FilterSaturation::FilterSaturation(float saturation)
	:
	Object(),
	fSaturation(saturation)
{
}

// destructor
FilterSaturation::~FilterSaturation()
{
}

// #pragma mark - BaseObject

// Clone
BaseObject*
FilterSaturation::Clone(CloneContext& context) const
{
	return new(std::nothrow) FilterSaturation(*this);
}


// DefaultName
const char*
FilterSaturation::DefaultName() const
{
	return "Saturation";
}

// AddProperties
void
FilterSaturation::AddProperties(PropertyObject* object, uint32 flags) const
{
	BaseObject::AddProperties(object, flags);

	object->AddProperty(new (std::nothrow) FloatProperty(
		PROPERTY_SATURATION, fSaturation, 0.0f, 10.0f));
}

// SetToPropertyObject
bool
FilterSaturation::SetToPropertyObject(const PropertyObject* object, uint32 flags)
{
	AutoNotificationSuspender _(this);
	BaseObject::SetToPropertyObject(object, flags);

	// filter radius
	SetSaturation(object->Value(PROPERTY_SATURATION, fSaturation));

	return HasPendingNotifications();
}

// #pragma mark -

// Snapshot
ObjectSnapshot*
FilterSaturation::Snapshot() const
{
	return new FilterSaturationSnapshot(this);
}

// #pragma mark -

// IsRegularTransformable
bool
FilterSaturation::IsRegularTransformable() const
{
	return false;
}

// SetSaturation
void
FilterSaturation::SetSaturation(float saturation)
{
	if (fSaturation == saturation)
		return;

	fSaturation = saturation;

	UpdateChangeCounter();
	InvalidateParent();
	Notify();
}

