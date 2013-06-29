/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "FilterDropShadow.h"

#include <new>

#include "Color.h"
#include "FilterDropShadowSnapshot.h"

// constructor
FilterDropShadow::FilterDropShadow()
	: Object()
	, fFilterRadius(3.0)
	, fOffsetX(0.0)
	, fOffsetY(0.0)
	, fOpacity(255.0)
	, fColorProvider(new (std::nothrow) ::Color(), true)
{
}

// constructor
FilterDropShadow::FilterDropShadow(float radius)
	: Object()
	, fFilterRadius(radius)
	, fOffsetX(0.0)
	, fOffsetY(0.0)
	, fOpacity(255.0)
	, fColorProvider(new (std::nothrow) ::Color(), true)
{
}

// destructor
FilterDropShadow::~FilterDropShadow()
{
}

// #pragma mark - BaseObject

// DefaultName
const char*
FilterDropShadow::DefaultName() const
{
	return "Drop shadow";
}

// AddProperties
void
FilterDropShadow::AddProperties(PropertyObject* object, uint32 flags) const
{
	Object::AddProperties(object, flags);

	object->AddProperty(new (std::nothrow) FloatProperty(
		PROPERTY_FILTER_RADIUS, fFilterRadius, 0.0f, 10000.0f));

	object->AddProperty(new (std::nothrow) FloatProperty(
		PROPERTY_OFFSET_X, fOffsetX, -10000.0f, 10000.0f));
	object->AddProperty(new (std::nothrow) FloatProperty(
		PROPERTY_OFFSET_Y, fOffsetY, -10000.0f, 10000.0f));

	object->AddProperty(new (std::nothrow) FloatProperty(
		PROPERTY_OPACITY, fOpacity, 0.0f, 255.0f));

	if (fColorProvider.Get() != NULL)
		fColorProvider->AddProperties(object, flags);
}

// SetToPropertyObject
bool
FilterDropShadow::SetToPropertyObject(const PropertyObject* object, uint32 flags)
{
	AutoNotificationSuspender _(this);
	Object::SetToPropertyObject(object, flags);

	// filter radius
	SetFilterRadius(object->Value(PROPERTY_FILTER_RADIUS, fFilterRadius));

	// offset
	SetOffsetX(object->Value(PROPERTY_OFFSET_X, fOffsetX));
	SetOffsetY(object->Value(PROPERTY_OFFSET_Y, fOffsetY));

	// opacity
	SetOpacity(object->Value(PROPERTY_OPACITY, fOpacity));

	if (fColorProvider.Get() != NULL) {
		if (fColorProvider->SetToPropertyObject(object, flags)) {
			Notify();
			UpdateChangeCounter();
			InvalidateParent();
		}
	}

	return HasPendingNotifications();
}

// #pragma mark -

// Snapshot
ObjectSnapshot*
FilterDropShadow::Snapshot() const
{
	return new (std::nothrow) FilterDropShadowSnapshot(this);
}

// #pragma mark -

// IsRegularTransformable
bool
FilterDropShadow::IsRegularTransformable() const
{
	return false;
}

// ExtendDirtyArea
void
FilterDropShadow::ExtendDirtyArea(BRect& area) const
{
	// "area" is the dirty area "below" this object.
	// This function should change the area so that
	// it includes other pixels in the bitmap that are
	// affected by this object, if pixels in the given
	// "area" change.
	if (fOpacity <= 0.0f)
		return;

	float extend = ceilf(fFilterRadius * Transformation().Scale()) + 3;
		// + 1 to be on the save side with regards
		// to pixel indices versus areas...
	BRect target(area);
	target.OffsetBy(fOffsetX, fOffsetY);
	target.InsetBy(-extend, -extend);
	area = area | target;
}

// SetFilterRadius
void
FilterDropShadow::SetFilterRadius(float filterRadius)
{
	_SetMember(fFilterRadius, filterRadius);
}

// SetOffsetX
void
FilterDropShadow::SetOffsetX(float offsetX)
{
	_SetMember(fOffsetX, offsetX);
}

// SetOffsetY
void
FilterDropShadow::SetOffsetY(float offsetY)
{
	_SetMember(fOffsetY, offsetY);
}

// SetOpacity
void
FilterDropShadow::SetOpacity(float opacity)
{
	_SetMember(fOpacity, opacity);
}

// #pragma mark -

// _SetMember
void
FilterDropShadow::_SetMember(float& member, float value)
{
	if (member == value)
		return;

	member = value;

	UpdateChangeCounter();
	InvalidateParent();
	Notify();
}
