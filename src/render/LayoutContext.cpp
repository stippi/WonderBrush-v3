/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#include "LayoutContext.h"

#include <new>

#include <Debug.h>


using std::nothrow;

// constructor
LayoutContext::LayoutContext(LayoutState* initialState)
	: fCurrentState(initialState)
	, fZoomLevel(1.0)
{
}

// destructor
LayoutContext::~LayoutContext()
{
}

// Init
void
LayoutContext::Init(double zoomLevel)
{
	ASSERT(fCurrentState->Previous == NULL);

	fZoomLevel = zoomLevel;
	// set the zoom level on the inital LayoutState
	fCurrentState->Matrix.Reset();
	fCurrentState->Matrix.ScaleBy(B_ORIGIN, fZoomLevel, fZoomLevel);
}

// PushState
void
LayoutContext::PushState(LayoutState* state)
{
	ASSERT(state->Previous == fCurrentState);

	fCurrentState = state;
}

// PopState
void
LayoutContext::PopState()
{
	ASSERT(fCurrentState->Previous != NULL);

	fCurrentState = fCurrentState->Previous;
}

// SetTransformation
void
LayoutContext::SetTransformation(const Transformable& matrix)
{
	fCurrentState->Matrix = fCurrentState->Previous->Matrix;
	fCurrentState->Matrix.Multiply(matrix);
}

