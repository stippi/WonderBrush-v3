/*
 * Copyright 2007-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "StyleableSnapshot.h"

#include <stdio.h>

#include "AutoLocker.h"
#include "Paint.h"
	// TODO: Remove, put all the handling for Style into RenderEngine...
#include "Style.h"
#include "Styleable.h"

// constructor
StyleableSnapshot::StyleableSnapshot(const Styleable* styleable)
	: ObjectSnapshot(styleable)
	, fOriginal(styleable)
	, fStyle(*styleable->Style())
{
}

// destructor
StyleableSnapshot::~StyleableSnapshot()
{
}

// #pragma mark -

// Sync
bool
StyleableSnapshot::Sync()
{
	if (ObjectSnapshot::Sync()) {
		fStyle = *fOriginal->Style();
		return true;
	}
	return false;
}

// Render
void
StyleableSnapshot::Render(RenderEngine& engine, RenderBuffer* bitmap,
	BRect area) const
{
	// TODO: Setup the RenderEngine here (everything contained in Style).
	// Should this be part of another method? But then every descendant class
	// needs to call it. Would be nice if the calling code appeared only once.
}
