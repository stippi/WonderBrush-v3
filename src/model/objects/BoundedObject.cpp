/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>. All rights reserved.
 */

#include "BoundedObject.h"

// constructor
BoundedObject::BoundedObject()
	: Object()
	, fTransformedBounds(0, 0, -1, -1)
{
}

// constructor
BoundedObject::BoundedObject(const BoundedObject& other)
	: Object(other)
	, fTransformedBounds(other.fTransformedBounds)
{
}

// destructor
BoundedObject::~BoundedObject()
{
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

// NotifyAndUpdate
void
BoundedObject::NotifyAndUpdate()
{
	UpdateChangeCounter();
	UpdateBounds();
	Notify();
}

