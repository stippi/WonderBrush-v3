/*
 * Copyright 2018, Stephan Aßmus <superstippi@gmx.de>.
 * Distributed under the terms of the MIT License.
 */
#include "FilterBrightness.h"

#include <new>

#include "FilterBrightnessSnapshot.h"

// constructor
FilterBrightness::FilterBrightness()
	:
	Object(),
	fOffset(0),
	fFactor(1.0)
{
}

// constructor
FilterBrightness::FilterBrightness(const FilterBrightness& other)
	:
	Object(other),
	fOffset(other.fOffset),
	fFactor(other.fFactor)
{
}

// constructor
FilterBrightness::FilterBrightness(int32 offset, float factor)
	:
	Object(),
	fOffset(offset),
	fFactor(factor)
{
}

// destructor
FilterBrightness::~FilterBrightness()
{
}

// #pragma mark - BaseObject

// Clone
BaseObject*
FilterBrightness::Clone(CloneContext& context) const
{
	return new(std::nothrow) FilterBrightness(*this);
}


// DefaultName
const char*
FilterBrightness::DefaultName() const
{
	return "Brightness";
}

// AddProperties
void
FilterBrightness::AddProperties(PropertyObject* object, uint32 flags) const
{
	BaseObject::AddProperties(object, flags);

	object->AddProperty(new (std::nothrow) IntProperty(
		PROPERTY_BRIGHTNESS_OFFSET, fOffset, -128, 128));
	object->AddProperty(new (std::nothrow) FloatProperty(
		PROPERTY_BRIGHTNESS_FACTOR, fFactor, 0.0f, 10.0f));
}

// SetToPropertyObject
bool
FilterBrightness::SetToPropertyObject(const PropertyObject* object, uint32 flags)
{
	AutoNotificationSuspender _(this);
	BaseObject::SetToPropertyObject(object, flags);

	// offset & factor
	SetOffset(object->Value(PROPERTY_BRIGHTNESS_OFFSET, fOffset));
	SetFactor(object->Value(PROPERTY_BRIGHTNESS_FACTOR, fFactor));

	return HasPendingNotifications();
}

// #pragma mark -

// Snapshot
ObjectSnapshot*
FilterBrightness::Snapshot() const
{
	return new FilterBrightnessSnapshot(this);
}

// #pragma mark -

// IsRegularTransformable
bool
FilterBrightness::IsRegularTransformable() const
{
	return false;
}

// SetOffset
void
FilterBrightness::SetOffset(int32 offset)
{
	if (fOffset == offset)
		return;

	fOffset = offset;

	UpdateChangeCounter();
	InvalidateParent();
	Notify();
}

// SetFactor
void
FilterBrightness::SetFactor(float factor)
{
	if (fFactor == factor)
		return;

	fFactor = factor;

	UpdateChangeCounter();
	InvalidateParent();
	Notify();
}

