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
	UpdateChangeCounter();
	UpdateBounds();
}

// #pragma mark -

// UpdateBounds
void
BoundedObject::UpdateBounds()
{
	Transformable globalTransform = Transformation();
	BRect newTransformedBounds = globalTransform.TransformBounds(Bounds());

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


