/*
 * Copyright 2007 - 2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#include "Object.h"

#include "Layer.h"


// constructor
Object::Object()
	: Transformable()
	, fChangeCounter(0)
	, fParent(NULL)
{
}

// destructor
Object::~Object()
{
}

// #pragma mark -

// SetParent
void
Object::SetParent(Layer* layer)
{
	fParent = layer;
}

// Level
int32
Object::Level() const
{
	if (!fParent)
		return 0;
	return fParent->Level() + 1;
}

// #pragma mark -

// SetName
void
Object::SetName(const char* name)
{
	if (!name || fName == name)
		return;

	fName = name;
	if (fParent)
		fParent->ObjectChanged(this);
}

// Name
const char*
Object::Name() const
{
	if (fName.Length() > 0)
		return fName.String();
	return DefaultName();
}

// GetIcon
bool
Object::GetIcon(const BBitmap* bitmap) const
{
	return false;
}

// Transformation
Transformable
Object::Transformation() const
{
	Transformable t;
	if (fParent)
		t = fParent->Transformation();
	t.Multiply(*this);
	return t;
}

// IsRegularTransformable
bool
Object::IsRegularTransformable() const
{
	return true;
}

// #pragma mark -

// ExtendDirtyArea
void
Object::ExtendDirtyArea(BRect& area) const
{
	// "area" is the dirty area "below" this object.
	// This function should change the area so that
	// it includes other pixels in the bitmap that are
	// affected by this object, if pixels in the given
	// "area" change.
}

// InvalidateParent
void
Object::InvalidateParent(const BRect& area)
{
	if (fParent)
		fParent->Invalidate(area, fParent->IndexOf(this));
}

// UpdateChangeCounter
void
Object::UpdateChangeCounter()
{
	if (fParent)
		fParent->UpdateChangeCounter();
	fChangeCounter++;
}


