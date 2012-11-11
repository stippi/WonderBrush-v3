/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "BrushStroke.h"

#include "support.h"

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
	, fPaint(new(std::nothrow) ::Paint((rgb_color){ 0, 0, 0, 255 }), true)
{
	if (fPaint.Get() != NULL)
		fPaint->AddListener(this);
}

// destructor
BrushStroke::~BrushStroke()
{
	SetBrush(NULL);
	SetPaint(NULL);
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

// #pragma mark -

// AddProperties
void
BrushStroke::AddProperties(PropertyObject* object, uint32 flags) const
{
	BoundedObject::AddProperties(object, flags);
	fPaint->AddProperties(object, flags);
}

// SetToPropertyObject
bool
BrushStroke::SetToPropertyObject(const PropertyObject* object, uint32 flags)
{
	AutoNotificationSuspender _(this);

	BoundedObject::SetToPropertyObject(object, flags);

	fPaint->SetToPropertyObject(object, flags);

	return HasPendingNotifications();
}

// #pragma mark -

// HitTest
bool
BrushStroke::HitTest(const BPoint& canvasPoint)
{
	int32 count = fStroke.CountObjects();
	if (count == 0 || !TransformedBounds().Contains(canvasPoint))
		return false;

	BPoint objectPoint(canvasPoint);
	Transformation().InverseTransform(&objectPoint);

	const StrokePoint* previous = fStroke.ObjectAt(0);
	const float radius = max_c(fBrush->MinRadius(), fBrush->MaxRadius());
	if (count > 1) {
		for (int32 i = 1; i < count; i++) {
			const StrokePoint* current = fStroke.ObjectAt(i);

			float dist = point_stroke_distance(previous->point, current->point,
				objectPoint, radius);
			if (dist < radius)
				return true;

			previous = current;
		}
	} else if (previous != NULL) {
		float dist = point_point_distance(previous->point, objectPoint);
		if (dist < radius)
			return true;
	}

	return false;
}

// #pragma mark -

// Bounds
BRect
BrushStroke::Bounds()
{
	BRect bounds(LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN);

	if (fBrush.Get() == NULL)
		return bounds;

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

void
BrushStroke::ObjectChanged(const Notifier* object)
{
	if (object == fPaint.Get()) {
		// Forward notification
		NotifyAndUpdate();
	}
}

// #pragma mark -

// SetBrush
void
BrushStroke::SetBrush(::Brush* brush)
{
	if (fBrush.Get() == brush)
		return;

	if (fBrush.SetTo(brush))
		NotifyAndUpdate();
}

// SetPaint
void
BrushStroke::SetPaint(::Paint* paint)
{
	if (fPaint.Get() == paint)
		return;

	if (fPaint.Get() != NULL)
		fPaint->RemoveListener(this);

	fPaint.SetTo(paint);

	if (fPaint.Get() != NULL) {
		fPaint->AddListener(this);
		ObjectChanged(paint);
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
