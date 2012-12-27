/*
 * Copyright 2007-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "Rect.h"

#include "RectSnapshot.h"
#include "RenderEngine.h"

// constructor
Rect::Rect()
	: Styleable()
	, fArea(10, 10, 60, 60)
{
	InitBounds();
}

// constructor
Rect::Rect(const BRect& area, const rgb_color& color)
	: Styleable(color)
	, fArea(area)
{
	InitBounds();
}

// destructor
Rect::~Rect()
{
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
Rect::HitTest(const BPoint& canvasPoint)
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

	UpdateChangeCounter();
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
	BRect bounds = fArea;
	Style()->ExtendBounds(bounds);
	return bounds;
}

