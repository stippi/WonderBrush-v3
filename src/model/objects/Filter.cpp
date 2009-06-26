/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#include "Filter.h"

#include <new>

#include "FilterSnapshot.h"

// constructor
Filter::Filter()
	:
	Object(),
	fFilterRadius(20.0)
{
}

// constructor
Filter::Filter(float radius)
	:
	Object(),
	fFilterRadius(radius)
{
}

// destructor
Filter::~Filter()
{
}

// #pragma mark - BaseObject

// DefaultName
const char*
Filter::DefaultName() const
{
	return "Gaussian Blur";
}

// MakePropertyObject
PropertyObject*
Filter::MakePropertyObject() const
{
	PropertyObject* object = BaseObject::MakePropertyObject();
	if (object == NULL)
		return NULL;

	object->AddProperty(new (std::nothrow) FloatProperty(
		PROPERTY_FILTER_RADIUS, fFilterRadius, 0.0f, 10000.0f));

	return object;
}

// SetToPropertyObject
bool
Filter::SetToPropertyObject(const PropertyObject* object)
{
	AutoNotificationSuspender _(this);
	BaseObject::SetToPropertyObject(object);

	// filter radius
	SetFilterRadius(object->Value(PROPERTY_FILTER_RADIUS, fFilterRadius));

	return HasPendingNotifications();
}

// #pragma mark -

// Snapshot
ObjectSnapshot*
Filter::Snapshot() const
{
	return new FilterSnapshot(this);
}

// #pragma mark -

// IsRegularTransformable
bool
Filter::IsRegularTransformable() const
{
	return false;
}

// ExtendDirtyArea
void
Filter::ExtendDirtyArea(BRect& area) const
{
	// "area" is the dirty area "below" this object.
	// This function should change the area so that
	// it includes other pixels in the bitmap that are
	// affected by this object, if pixels in the given
	// "area" change.

	float extend = ceilf(fFilterRadius) + 3;
		// + 1 to be on the save side with regards
		// to pixel indices versus areas...
	area.InsetBy(-extend, -extend);
}

// SetFilterRadius
void
Filter::SetFilterRadius(float filterRadius)
{
	if (fFilterRadius == filterRadius)
		return;

	fFilterRadius = filterRadius;

	UpdateChangeCounter();
	InvalidateParent();
	Notify();
}


