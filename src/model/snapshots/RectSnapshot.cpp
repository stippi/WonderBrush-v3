/*
 * Copyright 2007-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#include "RectSnapshot.h"

#include <stdio.h>

#include "support.h"

#include "Rect.h"
#include "RenderEngine.h"

// constructor
RectSnapshot::RectSnapshot(const Rect* rect)
	: StyleableSnapshot(rect)
	, fOriginal(rect)
	, fArea(rect->Area())
	, fRoundCornerRadius(rect->RoundCornerRadius())
{
}

// destructor
RectSnapshot::~RectSnapshot()
{
}

// #pragma mark -

// Original
const Object*
RectSnapshot::Original() const
{
	return fOriginal;
}

// Sync
bool
RectSnapshot::Sync()
{
	if (StyleableSnapshot::Sync()) {
		fArea = fOriginal->Area();
		fRoundCornerRadius = fOriginal->RoundCornerRadius();
		return true;
	}
	return false;
}

// Render
void
RectSnapshot::Render(RenderEngine& engine, RenderBuffer* bitmap,
	BRect area) const
{
	engine.SetStyle(fStyle);
	engine.SetTransformation(LayoutedState().Matrix);

	double radius = fRoundCornerRadius;
	if (radius <= 0.0)
		radius = 0.0;
	engine.DrawRectangle(fArea, area, radius, radius);
}


