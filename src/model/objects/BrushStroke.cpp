/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "BrushStroke.h"

#include "BrushStrokeSnapshot.h"
#include "RenderBuffer.h"
#include "RenderEngine.h"

// constructor
StrokePoint::StrokePoint()
	: point(0.0f, 0.0f)
	, pressure(1.0f)
	, tiltX(0.0f)
	, tiltY(0.0f)
{
}

// constructor
StrokePoint::StrokePoint(const BPoint& point, float pressure,
		float tiltX, float tiltY)
	: point(point)
	, pressure(pressure)
	, tiltX(tiltX)
	, tiltY(tiltY)
{
}

// constructor
StrokePoint::StrokePoint(const StrokePoint& other)
{
	*this = other;
}

// operator=
StrokePoint&
StrokePoint::operator=(const StrokePoint& other)
{
	point = other.point;
	pressure = other.pressure;
	tiltX = other.tiltX;
	tiltY = other.tiltY;

	return *this;
}

// operator==
bool
StrokePoint::operator==(const StrokePoint& other) const
{
	return point == other.point
		&& pressure == other.pressure
		&& tiltX == other.tiltX
		&& tiltY == other.tiltY;
}

// operator!=
bool
StrokePoint::operator!=(const StrokePoint& other) const
{
	return !(*this == other);
}

// #pragma mark -

// constructor
BrushStroke::BrushStroke()
	: BoundedObject()
	, fBrush(new(std::nothrow) ::Brush(), true)
{
}

// destructor
BrushStroke::~BrushStroke()
{
	SetBrush(NULL);
}

// #pragma mark -

// Snapshot
ObjectSnapshot*
BrushStroke::Snapshot() const
{
	return new(std::nothrow) BrushStrokeSnapshot(this);
}

// DefaultName
const char*
BrushStroke::DefaultName() const
{
	return "Brush stroke";
}

// HitTest
bool
BrushStroke::HitTest(const BPoint& canvasPoint) const
{
	// TODO
	return false;
}

// #pragma mark -

// Bounds
BRect
BrushStroke::Bounds()
{
	BRect bounds(LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN);
	uint32 count = fStroke.CountObjects();
	for (uint32 i = 0; i < count; i++) {
		StrokePoint* point = fStroke.ObjectAtFast(i);

		float radius = fBrush->Radius(point->pressure);
		BRect brushBounds(
			point->point.x - radius,
			point->point.y - radius,
			point->point.x + radius,
			point->point.y + radius);

		bounds = bounds | brushBounds;
	}
	return bounds;
}

// #pragma mark -

// SetBrush
void
BrushStroke::SetBrush(::Brush* brush)
{
	if (fBrush.Get() == brush)
		return;

	if (fBrush.SetTo(brush)) {
		UpdateBounds();
		UpdateChangeCounter();
		Notify();
	}
}

// AppendPoint
bool
BrushStroke::AppendPoint(const StrokePoint& point)
{
	BRect invalid(LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN);
	if (StrokePoint* lastPoint = fStroke.LastObject()) {
		invalid.Set(lastPoint->point.x, lastPoint->point.y,
			lastPoint->point.x, lastPoint->point.y);
		float radius = fBrush->Radius(lastPoint->pressure);
		invalid.InsetBy(-radius, -radius);
	}

	if (!fStroke.AppendObject(point)) {
		fprintf(stderr, "BrushStroke::AppendPoint(): Failed to add "
			"tracking point to BrushStroke. Out of memory\n");
		return false;
	}

	float radius = fBrush->Radius(point.pressure);
	invalid = invalid
		| (BRect(point.point.x, point.point.y, point.point.x, point.point.y)
			.InsetBySelf(-radius, -radius));

	// Reset transformed bounds without invalidation, invalidate only
	// changed region.
	InitBounds();
	UpdateChangeCounter();
	Notify();

	InvalidateParent(invalid);

	return true;
}
