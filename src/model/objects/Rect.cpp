/*
 * Copyright 2007-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "Rect.h"

#include "RectSnapshot.h"
#include "RenderEngine.h"

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
	: Styleable()
	, fArea(10, 10, 60, 60)
	, fListeners(4)
{
	InitBounds();
}

// constructor
Rect::Rect(const BRect& area, const rgb_color& color)
	: Styleable(color)
	, fArea(area)
	, fListeners(4)
{
	InitBounds();
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

// HitTest
bool
Rect::HitTest(const BPoint& canvasPoint) const
{
	RenderEngine engine(Transformation());
	return engine.HitTest(fArea, canvasPoint);
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

	UpdateBounds();
}

// Area
BRect
Rect::Area() const
{
	return fArea;
}

// Bounds
BRect
Rect::Bounds()
{
	return fArea;
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

