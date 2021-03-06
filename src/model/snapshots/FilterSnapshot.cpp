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

#include <agg_blur.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rendering_buffer.h>

#include "Filter.h"
#include "GaussFilter.h"
#include "LayoutContext.h"
#include "RenderBuffer.h"
#include "StackBlurFilter.h"


// constructor
FilterSnapshot::FilterSnapshot(const Filter* filter)
	: ObjectSnapshot(filter)
	, fOriginal(filter)
	, fFilterRadius(filter->FilterRadius())
	, fLayoutedFilterRadius(fFilterRadius)
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

// Layout
void
FilterSnapshot::Layout(LayoutContext& context, uint32 flags)
{
	ObjectSnapshot::Layout(context, flags);
	fLayoutedFilterRadius = fFilterRadius * LayoutedState().Matrix.Scale();
}

// Render
void
FilterSnapshot::Render(RenderEngine& engine, RenderBuffer* bitmap,
	BRect area) const
{
	float extend = ceilf(fLayoutedFilterRadius) + 3;
	BRect source = area;
	source.InsetBy(-extend, -extend);

	RenderBuffer buffer(bitmap, source, false);

// TODO: GaussFilter is broken! Using only the StackBlurFilter here means that
// radius is clamped to 254.
//	if (fLayoutedFilterRadius < 254) {
		// stack blur is really fast and independent
		// of the filter radius, but limited to 254
		// filter radius maximum
//		StackBlurFilter filter;
//		filter.FilterRGBA64(&buffer, fLayoutedFilterRadius);
//	} else {
//		// gauss filter supports any radius and is
//		// independent of the radius as well, but
//		// uses floating point math/conversion, so
//		// it is slower
//		GaussFilter filter;
//		filter.FilterRGBA64(&buffer, fLayoutedFilterRadius);
//	}

	agg::rendering_buffer aggBuffer;
	aggBuffer.attach(buffer.Bits(), buffer.Width(), buffer.Height(),
		buffer.BytesPerRow());
	agg::pixfmt_bgra64_pre pixelFormat(aggBuffer);
	
	agg::stack_blur<agg::rgba16, agg::stack_blur_calc_rgba<> > stackBlur;
	stackBlur.blur(pixelFormat, agg::uround(fLayoutedFilterRadius));

//	agg::recursive_blur<agg::rgba16, agg::recursive_blur_calc_rgba<> > recursiveBlur;
//	recursiveBlur.blur(pixelFormat, fLayoutedFilterRadius);

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

	float extend = ceilf(fLayoutedFilterRadius) + 3;
		// + 3 to account for the required 3 edge pixles
	area.InsetBy(-extend, -extend);
}

