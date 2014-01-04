/*
 * Copyright 2007 - 2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#include "Object.h"

#include <stdio.h>

#include "Layer.h"


// constructor
Object::Object()
	:
	Transformable(),
	BaseObject(),
	fChangeCounter(0),
	fParent(NULL),
	fIsVisible(true)
{
}

// constructor
Object::Object(const Object& other)
	:
	Transformable(other),
	BaseObject(other),
	fChangeCounter(0),
	fParent(NULL),
	fIsVisible(other.fIsVisible)
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
	UpdateChangeCounter();
}

// Level
int32
Object::Level() const
{
	if (fParent == NULL)
		return 0;
	return fParent->Level() + 1;
}

// SetVisible
void
Object::SetVisible(bool visible)
{
	if (fIsVisible != visible) {
		fIsVisible = visible;
		NotifyListeners();
		UpdateChangeCounter();
		InvalidateParent();
	}
}

// Assets
AssetList
Object::Assets() const
{
	return AssetList();
}

// #pragma mark -

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
	t.PreMultiply(*this);
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

// InvalidateParent
void
Object::InvalidateParent()
{
	if (fParent)
		fParent->Invalidate(fParent->Bounds(), fParent->IndexOf(this));
}

// HitTest
bool
Object::HitTest(const BPoint& canvasPoint)
{
	return false;
}

// UpdateChangeCounter
void
Object::UpdateChangeCounter()
{
	if (fParent)
		fParent->UpdateChangeCounter();
	fChangeCounter++;
}

// #pragma mark -

// NotifyListeners
void
Object::NotifyListeners()
{
	if (fParent != NULL)
		fParent->ObjectChanged(this);
	BaseObject::NotifyListeners();
}

// TransformationChanged
void
Object::TransformationChanged()
{
	NotifyListeners();
	UpdateChangeCounter();
	InvalidateParent();
}

