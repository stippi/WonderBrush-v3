/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "LayoutState.h"

// constructor
LayoutState::LayoutState()
	: Previous(NULL)
	, Matrix()
	// TODO: Default to global Null paints! (black/white or whatever)
	, fFillPaint(NULL)
	, fStrokePaint(NULL)
{
}

// constructor
LayoutState::LayoutState(LayoutState* previous)
	: Previous(previous)
	, Matrix(previous->Matrix)
	, fFillPaint(NULL)
	, fStrokePaint(NULL)
{
}

// destructor
LayoutState::~LayoutState()
{
	SetFillPaint(NULL);
	SetStrokePaint(NULL);
}

// operator=
LayoutState&
LayoutState::operator=(const LayoutState& other)
{
	Previous = other.Previous;
	Matrix = other.Matrix;
	SetFillPaint(other.fFillPaint);
	SetStrokePaint(other.fStrokePaint);

	return *this;
}

// SetFillPaint
void
LayoutState::SetFillPaint(Paint* paint)
{
	_SetPaint(fFillPaint, paint);
}

// SetStrokePaint
void
LayoutState::SetStrokePaint(Paint* paint)
{
	_SetPaint(fStrokePaint, paint);
}

// _SetPaint
void
LayoutState::_SetPaint(Paint*& member, Paint* paint)
{
	// The theory here is that the paint objects held by a LayoutState
	// are always only additional references to existing paint objects
	// that never change through the LayoutState. So all the LayoutState
	// needs to do is to add/remove references.
	if (member == paint)
		return;

	if (member != NULL)
		member->RemoveReference();

	if (paint != NULL)
		paint->AddReference();

	member = paint;
}

