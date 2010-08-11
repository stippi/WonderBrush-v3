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
	: ObjectSnapshot(stroke)
	, fOriginal(stroke)
	, fBrush()
{
	_Sync();
}

// destructor
BrushStrokeSnapshot::~BrushStrokeSnapshot()
{
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
	if (ObjectSnapshot::Sync()) {
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
	engine.SetTransformation(LayoutedState().Matrix);
	// TODO: ...
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

	fStroke = fOriginal->Stroke();
}
