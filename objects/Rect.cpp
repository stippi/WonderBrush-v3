/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#include "Rect.h"

#include "RectSnapshot.h"

// constructor
RectListener::RectListener()
{
}

// destructor
RectListener::~RectListener()
{
}

// AreaChanged
void
RectListener::AreaChanged(Rect* rect, const BRect& oldArea,
	const BRect& newArea)
{
}

// Deleted
void
RectListener::Deleted(Rect* rect)
{
}

// #pragma mark -

// constructor
Rect::Rect()
	: Object()
	, fArea(10, 10, 60, 60)
	, fColor((rgb_color){ 0, 0, 0, 255 })
	, fListeners(4)
{
}

// constructor
Rect::Rect(const BRect& area, const rgb_color& color)
	: Object()
	, fArea(area)
	, fColor(color)
	, fListeners(4)
{
}

// destructor
Rect::~Rect()
{
	_NotifyDeleted();
}

// #pragma mark -

// Snapshot
ObjectSnapshot*
Rect::Snapshot() const
{
	return new RectSnapshot(this);
}

// DefaultName
const char*
Rect::DefaultName() const
{
	return "Rect";
}

// #pragma mark -

// SetArea
void
Rect::SetArea(const BRect& area)
{
	if (area == fArea)
		return;

	BRect oldArea(fArea);
	fArea = area;

	_NotifyAreaChanged(oldArea, fArea);
}

// Area
BRect
Rect::Area() const
{
	return fArea;
}

// SetColor
void
Rect::SetColor(const rgb_color& color)
{
//	if (color == fColor)
//		return;

//	rgb_color oldColor(fColor);
	fColor = color;

	UpdateChangeCounter();
//	_NotifyColorChanged(oldColor, fColor);
}

// Area
rgb_color
Rect::Color() const
{
	return fColor;
}

// #pragma mark -

// AddListener
bool
Rect::AddListener(RectListener* listener)
{
	if (!listener || fListeners.HasItem(listener))
		return false;
	return fListeners.AddItem(listener);
}

// RemoveListener
void
Rect::RemoveListener(RectListener* listener)
{
	fListeners.RemoveItem(listener);
}

// #pragma mark -

// _NotifyAreaChanged
void
Rect::_NotifyAreaChanged(const BRect& oldArea, const BRect& newArea)
{
	UpdateChangeCounter();

	int32 count = fListeners.CountItems();
	if (count == 0)
		return;

	BList listeners(fListeners);
	for (int32 i = 0; i < count; i++) {
		RectListener* listener = (RectListener*)listeners.ItemAtFast(i);
		listener->AreaChanged(this, oldArea, newArea);
	}
}

// _NotifyDeleted
void
Rect::_NotifyDeleted()
{
	int32 count = fListeners.CountItems();
	if (count == 0)
		return;

	BList listeners(fListeners);
	for (int32 i = 0; i < count; i++) {
		RectListener* listener = (RectListener*)listeners.ItemAtFast(i);
		listener->Deleted(this);
	}
}

