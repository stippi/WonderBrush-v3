/*
 * Copyright 2007-2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "Shape.h"

#include "Paint.h"
#include "ShapeSnapshot.h"
#include "Style.h"
#include "ui_defines.h"

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
	:
	Object(),
	fArea(10, 10, 60, 60),
	fStyle(new ::Style(), true),
	fListeners(4)
{
	Paint* paint = new Paint(kBlack);
	Reference<Paint> _(paint, true);
	fStyle->SetFillPaint(paint);
}

// constructor
Shape::Shape(const BRect& area, const rgb_color& color)
	:
	Object(),
	fArea(area),
	fStyle(new ::Style(), true),
	fListeners(4)
{
	Paint* paint = new Paint(color);
	Reference<Paint> _(paint, true);
	fStyle->SetFillPaint(paint);
}

// destructor
Shape::~Shape()
{
	_NotifyDeleted();
	fStyle->RemoveReference();
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

	InvalidateParent(oldArea | fArea);
}

// Area
BRect
Shape::Area() const
{
	return fArea;
}

// SetColor
void
Shape::SetStyle(::Style* style)
{
	if (fStyle.SetTo(style))
		InvalidateParent(fArea);
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

