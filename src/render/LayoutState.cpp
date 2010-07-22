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
	, fStrokeProperties(NULL)
{
}

// constructor
LayoutState::LayoutState(LayoutState* previous)
	: Previous(previous)
	, Matrix(previous->Matrix)
	, fFillPaint(NULL)
	, fStrokePaint(NULL)
	, fStrokeProperties(NULL)
{
	SetFillPaint(previous->fFillPaint);
	SetStrokePaint(previous->fStrokePaint);
	SetStrokeProperties(previous->fStrokeProperties);
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
	SetStrokeProperties(other.fStrokeProperties);

	return *this;
}

// SetFillPaint
void
LayoutState::SetFillPaint(Paint* paint)
{
	_SetMember(fFillPaint, paint);
}

// SetStrokePaint
void
LayoutState::SetStrokePaint(Paint* paint)
{
	_SetMember(fStrokePaint, paint);
}

// SetStrokeProperties
void
LayoutState::SetStrokeProperties(::StrokeProperties* properties)
{
	_SetMember(fStrokeProperties, properties);
}

// _SetMember
template <typename MemberType>
void
LayoutState::_SetMember(MemberType*& member, MemberType* newMember)
{
	// The theory here is that the objects held by a LayoutState are
	// always only additional references to existing objects that never
	// change through the LayoutState. So all the LayoutState needs to do
	// is to add/remove references.
	if (member == newMember)
		return;

	if (member != NULL)
		member->RemoveReference();

	if (newMember != NULL)
		newMember->AddReference();

	member = newMember;
}

