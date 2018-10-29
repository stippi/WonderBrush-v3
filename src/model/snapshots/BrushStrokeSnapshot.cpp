/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#include "BrushStrokeSnapshot.h"

#include <stdio.h>

#include "RenderBuffer.h"
#include "RenderEngine.h"

// constructor
BrushStrokeSnapshot::BrushStrokeSnapshot(const BrushStroke* stroke)
	: BoundedObjectSnapshot(stroke)
	, fOriginal(stroke)
	, fBrush()
	, fPaint(NULL)

	// TODO: Move this into Brush?
	, fMaxSpacing(0.1f)
{
	_Sync();
}

// destructor
BrushStrokeSnapshot::~BrushStrokeSnapshot()
{
	Paint::PaintCache().Put(fPaint);
}

// #pragma mark -

// Original
const Object*
BrushStrokeSnapshot::Original() const
{
	return fOriginal;
}

// Sync
bool
BrushStrokeSnapshot::Sync()
{
	if (BoundedObjectSnapshot::Sync()) {
		_Sync();
		return true;
	}
	return false;
}

// Render
void
BrushStrokeSnapshot::Render(RenderEngine& engine, RenderBuffer* bitmap,
	BRect area) const
{
	if (!Transformable::IsValid())
		return;
	engine.SetTransformation(LayoutedState().Matrix);

	area = bitmap->Bounds() & area;

	// TODO: Move actual Brush drawing into RenderEngine.
	uint8* dest = (uint8*)engine.AlphaBuffer().buf();
	uint32 bpr = engine.AlphaBuffer().stride();
	if (dest == NULL)
		return;

	engine.ClearAlphaBufferScanlines();

	// traverse lines
	float stepDistLeftOver = 0.0f;
	const StrokePoint* previous = fStroke.ObjectAt(0);
	int32 count = fStroke.CountObjects();
	bool drawnAnything = false;
	if (count > 1) {
		for (int32 i = 1; i < count; i++) {
			const StrokePoint* current = fStroke.ObjectAt(i);
			drawnAnything |= _StrokeLine(previous, current, dest, bpr,
				area, stepDistLeftOver);
			previous = current;
		}
	} else if (previous != NULL) {
		drawnAnything = _StrokeLine(previous, previous, dest, bpr,
			area, stepDistLeftOver);
	}
	if (drawnAnything) {
		// Blend alpha map with our paint
		engine.SetFillPaint(fPaint);
		engine.RenderAlphaBufferScanlines();
	}
}

// #pragma mark -

// _Sync
void
BrushStrokeSnapshot::_Sync()
{
	if (fOriginal->Brush() != NULL)
		fBrush = *fOriginal->Brush();
	else
		fBrush = Brush();

	if (fOriginal->Paint() != NULL) {
		// We can compare the SharedPaint pointers, since the cache should
		// not hand out different pointers for the same visual paint.
		fPaint = Paint::PaintCache().Get(*fOriginal->Paint());
	} else {
		Paint::PaintCache().Put(fPaint);
		fPaint = NULL;
	}

	fStroke = fOriginal->Stroke();
}

// _StepDist
float
BrushStrokeSnapshot::_StepDist(float scale) const
{
	float minStep;
	if (scale != 0.0f)
		minStep = 1.0f / scale;
	else
		minStep = 1.0f;

	float step = fBrush.MaxRadius() * 2.0 * fMaxSpacing;
	if (step < minStep)
		step = minStep;

	return step;
}

// _StrokeLine
bool
BrushStrokeSnapshot::_StrokeLine(const StrokePoint* a,
	const StrokePoint* b, uint8* dest, uint32 bpr,
	const BRect& constrainRect, float& stepDistLeftOver) const
{
	if (a == b) {
		BPoint p = a->point;
		fBrush.Draw(p, a->pressure, a->tiltX, a->tiltY, dest, bpr, *this,
			constrainRect);
		return true;
	}

//printf("_StrokeLine(): (%.1f, %.1f) -> (%.1f, %.1f)\n",
//	a->point.x, a->point.y, b->point.x, b->point.y);

	bool drawnAnything = false;

	const BPoint& pA = a->point;
	BPoint vector = b->point - pA;
	float dist = sqrtf(vector.x * vector.x + vector.y * vector.y);
	float pressureDiff = b->pressure - a->pressure;
	float tiltXDiff = b->tiltX - a->tiltX;
	float tiltYDiff = b->tiltY - a->tiltY;
	float scale = Scale();
	float stepDist = _StepDist(scale);
	float currentStepDist = stepDist;
	float minStepDist = scale != 0.0 ? 1.0 / scale : 1.0;
	if ((fBrush.Flags() & Brush::FLAG_PRESSURE_CONTROLS_RADIUS) != 0)
		currentStepDist = max_c(minStepDist, stepDist * a->pressure);
	float p = stepDistLeftOver != 0.0
		? currentStepDist - stepDistLeftOver : 0.0;
	if (p < dist) {
		for (; p < dist; p += currentStepDist) {
			float iterationScale = p / dist;
			float currentPressure = a->pressure
				+ pressureDiff * iterationScale;
			float currentTiltX = a->tiltX + tiltXDiff
				* iterationScale;
			float currentTiltY = a->tiltY + tiltYDiff
				* iterationScale;
			if ((fBrush.Flags() & Brush::FLAG_PRESSURE_CONTROLS_RADIUS) != 0) {
				currentStepDist = max_c(minStepDist,
					stepDist * currentPressure);
			}
			BPoint center(
				pA.x + vector.x * iterationScale,
				pA.y + vector.y * iterationScale);
			fBrush.Draw(center, currentPressure, currentTiltX, currentTiltY,
				dest, bpr, LayoutedState().Matrix, constrainRect);
		}
		stepDistLeftOver = dist - (p - currentStepDist);
		drawnAnything = true;
	} else
		stepDistLeftOver += dist;

	return drawnAnything;
}

