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
