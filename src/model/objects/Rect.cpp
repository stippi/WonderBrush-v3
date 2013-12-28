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
	, fArea(0, 0, 0, 0)
	, fRoundCornerRadius(0.0)
{
	InitBounds();
}

// constructor
Rect::Rect(const BRect& area, const rgb_color& color)
	: Styleable(color)
	, fArea(area)
	, fRoundCornerRadius(0.0)
{
	InitBounds();
}

// constructor
Rect::Rect(const Rect& other, CloneContext& context)
	: Styleable(other, context)
	, fArea(other.fArea)
	, fRoundCornerRadius(other.fRoundCornerRadius)
{
	InitBounds();
}

// destructor
Rect::~Rect()
{
}

// #pragma mark -

// Clone
BaseObject*
Rect::Clone(CloneContext& context) const
{
	return new(std::nothrow) Rect(*this, context);
}

// DefaultName
const char*
Rect::DefaultName() const
{
	return "Rect";
}

// #pragma mark -

// Snapshot
ObjectSnapshot*
Rect::Snapshot() const
{
	return new RectSnapshot(this);
}

// HitTest
bool
Rect::HitTest(const BPoint& canvasPoint)
{
	RenderEngine engine(Transformation());
	return engine.HitTest(fArea, canvasPoint);
}

// #pragma mark -

// Bounds
BRect
Rect::Bounds()
{
	BRect bounds = fArea;
	Style()->ExtendBounds(bounds);
	return bounds;
}

// #pragma mark -

// SetArea
void
Rect::SetArea(const BRect& area)
{
	if (area == fArea)
		return;

	fArea = area;

	NotifyAndUpdate();
}

// Area
BRect
Rect::Area() const
{
	return fArea;
}

// SetRoundCornerRadius
void
Rect::SetRoundCornerRadius(double radius)
{
	if (radius == fRoundCornerRadius)
		return;

	fRoundCornerRadius = radius;

	NotifyAndUpdate();
}

// RoundCornerRadius
double
Rect::RoundCornerRadius() const
{
	return fRoundCornerRadius;
}


