/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#include "RectSnapshot.h"

#include <stdio.h>

#include <Bitmap.h>

#include "support.h"

#include "Rect.h"
#include "RenderEngine.h"

// constructor
RectSnapshot::RectSnapshot(const Rect* rect)
	: StyleableSnapshot(rect)
	, fOriginal(rect)
	, fArea(rect->Area())
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
		return true;
	}
	return false;
}

// Render
void
RectSnapshot::Render(RenderEngine& engine, BBitmap* bitmap, BRect area) const
{
	engine.SetStyle(fStyle.Get());
	engine.SetTransformation(LayoutedState().Matrix);
	engine.DrawRectangle(fArea, area);
}


