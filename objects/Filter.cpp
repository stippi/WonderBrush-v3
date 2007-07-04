/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#include "Filter.h"

#include "FilterSnapshot.h"

// constructor
Filter::Filter()
	: Object()
	, fFilterRadius(20.0)
{
}

// constructor
Filter::Filter(float radius)
	: Object()
	, fFilterRadius(radius)
{
}

// destructor
Filter::~Filter()
{
}

// #pragma mark -

// Snapshot
ObjectSnapshot*
Filter::Snapshot() const
{
	return new FilterSnapshot(this);
}

// ExtendDirtyArea
void
Filter::ExtendDirtyArea(BRect& area) const
{
	// "area" is the dirty area "below" this object.
	// This function should change the area so that
	// it includes other pixels in the bitmap that are
	// affected by this object, if pixels in the given
	// "area" change.

	float extend = ceilf(fFilterRadius) + 3;
		// + 1 to be on the save side with regards
		// to pixel indices versus areas...
	area.InsetBy(-extend, -extend);
}

