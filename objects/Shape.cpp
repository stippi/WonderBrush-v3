/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#include "Shape.h"

#include "ShapeSnapshot.h"

// constructor
ShapeListener::ShapeListener()
{
}

// destructor
ShapeListener::~ShapeListener()
{
}

// AreaChanged
void
ShapeListener::AreaChanged(Shape* shape, const BRect& oldArea,
	const BRect& newArea)
{
}

// Deleted
void
ShapeListener::Deleted(Shape* shape)
{
}

// #pragma mark -

// constructor
Shape::Shape()
	: Object()
	, fArea(10, 10, 60, 60)
	, fColor((rgb_color){ 0, 0, 0, 255 })
	, fListeners(4)
{
}

// constructor
Shape::Shape(const BRect& area, const rgb_color& color)
	: Object()
	, fArea(area)
	, fColor(color)
	, fListeners(4)
{
}

// destructor
Shape::~Shape()
{
	_NotifyDeleted();
}

// #pragma mark -

// Snapshot
ObjectSnapshot*
Shape::Snapshot() const
{
	return new ShapeSnapshot(this);
}

// DefaultName
const char*
Shape::DefaultName() const
{
	return "Shape";
}

// #pragma mark -

// SetArea
void
Shape::SetArea(const BRect& area)
{
	if (area == fArea)
		return;

	BRect oldArea(fArea);
	fArea = area;

	_NotifyAreaChanged(oldArea, fArea);
}

// Area
BRect
Shape::Area() const
{
	return fArea;
}

// SetColor
void
Shape::SetColor(const rgb_color& color)
{
//	if (color == fColor)
//		return;

//	rgb_color oldColor(fColor);
	fColor = color;

//	_NotifyColorChanged(oldColor, fColor);
}

// Color
rgb_color
Shape::Color() const
{
	return fColor;
}

// #pragma mark -

// AddListener
bool
Shape::AddListener(ShapeListener* listener)
{
	if (!listener || fListeners.HasItem(listener))
		return false;
	return fListeners.AddItem(listener);
}

// RemoveListener
void
Shape::RemoveListener(ShapeListener* listener)
{
	fListeners.RemoveItem(listener);
}

// #pragma mark -

// _NotifyAreaChanged
void
Shape::_NotifyAreaChanged(const BRect& oldArea, const BRect& newArea)
{
	UpdateChangeCounter();

	int32 count = fListeners.CountItems();
	if (count == 0)
		return;

	BList listeners(fListeners);
	for (int32 i = 0; i < count; i++) {
		ShapeListener* listener = (ShapeListener*)listeners.ItemAtFast(i);
		listener->AreaChanged(this, oldArea, newArea);
	}
}

// _NotifyDeleted
void
Shape::_NotifyDeleted()
{
	int32 count = fListeners.CountItems();
	if (count == 0)
		return;

	BList listeners(fListeners);
	for (int32 i = 0; i < count; i++) {
		ShapeListener* listener = (ShapeListener*)listeners.ItemAtFast(i);
		listener->Deleted(this);
	}
}

