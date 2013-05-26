/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "LayoutState.h"

#include <stdio.h>

// constructor
LayoutState::LayoutState()
	: Previous(NULL)
	, Matrix()
	// TODO: Default to global Null paints! (black/white or whatever)
	, fFillPaint(NULL)
	, fStrokePaint(NULL)
	, fStrokeProperties(NULL)
{
	SetFillPaint(NULL);
	SetStrokePaint(NULL);
	SetStrokeProperties(NULL);
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
	SetStrokeProperties(NULL);
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
	if (paint != NULL)
		_SetMember(fFillPaint, *paint, Paint::PaintCache());
	else
		_SetMember(fFillPaint, Paint::EmptyPaint(), Paint::PaintCache());
}

// FillPaint
const Paint*
LayoutState::FillPaint() const
{
	return fFillPaint;
}

// SetStrokePaint
void
LayoutState::SetStrokePaint(Paint* paint)
{
	if (paint != NULL)
		_SetMember(fStrokePaint, *paint, Paint::PaintCache());
	else
		_SetMember(fStrokePaint, Paint::EmptyPaint(), Paint::PaintCache());
}

// StrokePaint
const Paint*
LayoutState::StrokePaint() const
{
	return fStrokePaint;
}

// SetStrokeProperties
void
LayoutState::SetStrokeProperties(::StrokeProperties* properties)
{
//	if (fStrokeProperties != NULL) {
//		printf("LayoutState::SetStrokeProperties(%p) %p -> %ld\n",
//			properties, fStrokeProperties, fStrokeProperties->CountReferences());
//	}
	if (properties != NULL) {
		_SetMember(fStrokeProperties, *properties,
			StrokeProperties::StrokePropertiesCache());
	} else {
		_SetMember(fStrokeProperties,
			StrokeProperties::EmptyStrokeProperties(),
			StrokeProperties::StrokePropertiesCache());
	}
//	if (fStrokeProperties != NULL) {
//		printf("                                (%p) %p -> %ld\n",
//			properties, fStrokeProperties, fStrokeProperties->CountReferences());
//	}
}

// StrokePaint
const ::StrokeProperties*
LayoutState::StrokeProperties() const
{
	return fStrokeProperties;
}

// _SetMember
template <typename MemberType, typename ValueType, typename CacheType>
void
LayoutState::_SetMember(MemberType*& member, const ValueType& newValue,
	CacheType& cache)
{
//	// The theory here is that the objects held by a LayoutState are
//	// always only additional references to existing objects that never
//	// change through the LayoutState. So all the LayoutState needs to do
//	// is to add/remove references.
//	if (member == newMember)
//		return;
//
//	if (newMember != NULL)
//		newMember->AddReference();
//
//	if (member != NULL)
//		member->RemoveReference();
//
//	member = newMember;
	if (member == NULL) {
		member = cache.Get(newValue);
		return;
	}

	if (*member == newValue)
		return;

	member = cache.PrepareForModifications(member);
	*member = newValue;
	member = cache.CommitModifications(member);
}

