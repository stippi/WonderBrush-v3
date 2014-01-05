/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>. All rights reserved.
 */

#include "BoundedObject.h"

// constructor
BoundedObject::BoundedObject()
	: Object()
	, fOpacity(255)
	, fTransformedBounds(0, 0, -1, -1)
{
}

// constructor
BoundedObject::BoundedObject(const BoundedObject& other)
	: Object(other)
	, fOpacity(other.fOpacity)
	, fTransformedBounds(other.fTransformedBounds)
{
}

// destructor
BoundedObject::~BoundedObject()
{
}

// AddProperties
void
BoundedObject::AddProperties(PropertyObject* object, uint32 flags) const
{
	Object::AddProperties(object, flags);

	IntProperty* opacity = new(std::nothrow) IntProperty(
		PROPERTY_OPACITY, fOpacity, 0, 255);
	if (opacity == NULL || !object->AddProperty(opacity)) {
		delete opacity;
		return;
	}
}

// SetToPropertyObject
bool
BoundedObject::SetToPropertyObject(const PropertyObject* object, uint32 flags)
{
	AutoNotificationSuspender _(this);

	Object::SetToPropertyObject(object, flags);

	SetOpacity(object->Value(PROPERTY_OPACITY, (int32)fOpacity));

	return HasPendingNotifications();
}

// TransformationChanged
void
BoundedObject::TransformationChanged()
{
	// Override the Object version, which invalidates the whole parent.
	// (That's actually valid for unbounded objects, such as filters.)
	NotifyListeners();
	UpdateChangeCounter();
	UpdateBounds();
}

// #pragma mark -

// InitBounds
void
BoundedObject::InitBounds()
{
	Transformable globalTransform = Transformation();
	fTransformedBounds = globalTransform.TransformBounds(Bounds());
}

// UpdateBounds
void
BoundedObject::UpdateBounds()
{
	Transformable globalTransform = Transformation();
	BRect newTransformedBounds = globalTransform.TransformBounds(Bounds());
	newTransformedBounds.left = floorf(newTransformedBounds.left) - 1.0f;
	newTransformedBounds.top = floorf(newTransformedBounds.top) - 1.0f;
	newTransformedBounds.right = ceilf(newTransformedBounds.right) + 1.0f;
	newTransformedBounds.bottom = ceilf(newTransformedBounds.bottom) + 1.0f;

	bool oldValid = fTransformedBounds.IsValid();
	bool newValid = newTransformedBounds.IsValid();

	if (oldValid && newValid)
		InvalidateParent(newTransformedBounds | fTransformedBounds);
	else if (oldValid)
		InvalidateParent(fTransformedBounds);
	else if (newValid)
		InvalidateParent(newTransformedBounds);

	fTransformedBounds = newTransformedBounds;

	// TODO: Notification would be nice?
}

// SetOpacity
void
BoundedObject::SetOpacity(uint8 opacity)
{
	if (fOpacity != opacity) {
		fOpacity = opacity;
		UpdateChangeCounter();
		NotifyListeners();
		InvalidateParent(fTransformedBounds);
	}
}

// NotifyAndUpdate
void
BoundedObject::NotifyAndUpdate()
{
	UpdateChangeCounter();
	UpdateBounds();
	Notify();
}

