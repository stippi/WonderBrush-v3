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
{
}

// destructor
LayoutContext::~LayoutContext()
{
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

