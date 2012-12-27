/*
 * Copyright 2007-2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "Shape.h"

#include "ShapeSnapshot.h"

// constructor
Shape::Shape()
	: Styleable()
	, fArea(10, 10, 60, 60)
{
	InitBounds();
}

// constructor
Shape::Shape(const BRect& area, const rgb_color& color)
	: Styleable(color)
	, fArea(area)
{
	InitBounds();
}

// destructor
Shape::~Shape()
{
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
Shape::AddProperties(PropertyObject* object, uint32 flags) const
{
	Styleable::AddProperties(object, flags);
}

// SetToPropertyObject
bool
Shape::SetToPropertyObject(const PropertyObject* object, uint32 flags)
{
	return Styleable::SetToPropertyObject(object, flags);
}

// DefaultName
const char*
Shape::DefaultName() const
{
	return "Shape";
}

// HitTest
bool
Shape::HitTest(const BPoint& canvasPoint)
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

	UpdateChangeCounter();
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
	BRect bounds = fArea;
	Style()->ExtendBounds(bounds);
	return bounds;
}

// #pragma mark -

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

