/*
 * Copyright 2007-2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
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
	: Styleable()
	, fArea(10, 10, 60, 60)
	, fListeners(4)
{
}

// constructor
Shape::Shape(const BRect& area, const rgb_color& color)
	: Styleable(color)
	, fArea(area)
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

// AddProperties
void
Shape::AddProperties(PropertyObject* object) const
{
	Styleable::AddProperties(object);
}

// SetToPropertyObject
bool
Shape::SetToPropertyObject(const PropertyObject* object)
{
	return Styleable::SetToPropertyObject(object);
}

// DefaultName
const char*
Shape::DefaultName() const
{
	return "Shape";
}

// HitTest
bool
Shape::HitTest(const BPoint& canvasPoint) const
{
	PathStorage path;
	_GetPath(path);
	RenderEngine engine(Transformation());
	return engine.HitTest(path, canvasPoint);
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

	UpdateBounds();
}

// Area
BRect
Shape::Area() const
{
	return fArea;
}

// Bounds
BRect
Shape::Bounds()
{
	return fArea;
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

// _GetPath
void
Shape::_GetPath(PathStorage& path) const
{
	path.move_to(fArea.left, fArea.top);
	path.line_to((fArea.left + fArea.right) / 2,
		fArea.top + fArea.Height() / 3);

	path.line_to(fArea.right, fArea.top);
	path.line_to(fArea.right - fArea.Width() / 3,
		(fArea.top + fArea.bottom) / 2);

	path.line_to(fArea.right, fArea.bottom);
	path.line_to((fArea.left + fArea.right) / 2,
		fArea.bottom - fArea.Height() / 3);

	path.line_to(fArea.left, fArea.bottom);
	path.line_to(fArea.left + fArea.Width() / 3,
		(fArea.top + fArea.bottom) / 2);
	path.close_polygon();
}

