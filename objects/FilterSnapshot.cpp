/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#include "FilterSnapshot.h"

#include <stdio.h>

#include <Bitmap.h>

#include "Filter.h"
#include "GaussFilter.h"
#include "RenderBuffer.h"
#include "StackBlurFilter.h"


// constructor
FilterSnapshot::FilterSnapshot(const Filter* filter)
	: ObjectSnapshot(filter)
	, fOriginal(filter)
	, fFilterRadius(filter->FilterRadius())
{
}

// destructor
FilterSnapshot::~FilterSnapshot()
{
}

// #pragma mark -

// Original
const Object*
FilterSnapshot::Original() const
{
	return fOriginal;
}

// Sync
bool
FilterSnapshot::Sync()
{
	if (ObjectSnapshot::Sync()) {
		fFilterRadius = fOriginal->FilterRadius();
		return true;
	}
	return false;
}

// Render
void
FilterSnapshot::Render(RenderEngine& engine, BBitmap* bitmap,
	BRect area) const
{
	float extend = ceilf(fFilterRadius) + 3;
	BRect source = area;
	source.InsetBy(-extend, -extend);

	RenderBuffer buffer(bitmap, source, false);

	if (fFilterRadius < 254) {
		// stack blur is really fast and independent
		// of the filter radius, but limited to 254
		// filter radius maximum
		StackBlurFilter filter;
		filter.FilterRGBA32(&buffer, fFilterRadius);
	} else {
		// gauss filter supports any radius and is
		// independent of the radius as well, but
		// uses floating point math/conversion, so
		// it is slower
		GaussFilter filter;
		filter.FilterRGB32(&buffer, fFilterRadius);
	}

	source.InsetBy(extend, extend);
	buffer.CopyTo(bitmap, source);
}

// RebuildAreaForDirtyArea
void
FilterSnapshot::RebuildAreaForDirtyArea(BRect& area) const
{
	// "area" is the area requested to be rendered by this
	// object.
	// This function should change the area so that
	// it includes all pixels outside the given area which
	// are required by this object to render the given area
	// correctly.

	float extend = ceilf(fFilterRadius) + 3;
		// + 3 to account for the required 3 edge pixles
	area.InsetBy(-extend, -extend);
}

