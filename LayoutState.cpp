/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#include "LayoutState.h"


// constructor
LayoutState::LayoutState()
	: Previous(NULL)
	, Matrix()
{
}

// constructor
LayoutState::LayoutState(LayoutState* previous)
	: Previous(previous)
	, Matrix(previous->Matrix)
{
}

// destructor
LayoutState::~LayoutState()
{
}
