/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>
 * Copyright 2007-2008, Christof Lutteroth, lutteroth@cs.auckland.ac.nz
 * Copyright 2007-2008, James Kim, jkim202@ec.auckland.ac.nz
 * Distributed under the terms of the MIT License.
 */

#include "ALMSpan.h"

#include <linprog/OperatorType.h>

#include "ALMLayout.h"
#include "ALMTab.h"


/**
 * The start boundary of the span.
 */
ALMTab*
ALMSpan::StartTab() const
{
	return fStartTab;
}


/**
 * The end boundary of the span.
 */
ALMTab*
ALMSpan::EndTab() const
{
	return fEndTab;
}


/**
 * Gets the span directly before this span.
 */
ALMSpan*
ALMSpan::Previous() const
{
	return fPrevious;
}


/**
 * Sets the span directly after this span.
 * May be null.
 */
void
ALMSpan::SetPrevious(ALMSpan* span)
{
	// If there should be no span directly before this span, then we have to
	// separate any such span and can remove any constraint that was used
	// to glue this span to it...
	if (span == NULL) {
		if (fPrevious == NULL)
			return;
		fPrevious->fNext = NULL;
		fPrevious->fNextGlue = NULL;
		fPrevious = NULL;
		delete fPreviousGlue;
		fPreviousGlue = NULL;
		return;
	}

	// ...otherwise we have to set up the pointers and the glue constraint
	// accordingly.
	if (span->fNext != NULL)
		span->SetNext(NULL);
	if (fPrevious != NULL)
		SetPrevious(NULL);

	fPrevious = span;
	fPrevious->fNext = this;
	span->fNextGlue = span->fEndTab->IsEqual(fStartTab);
	fPreviousGlue = span->fNextGlue;
}


/**
 * Gets the span directly below this span.
 */
ALMSpan*
ALMSpan::Next() const
{
	return fNext;
}


/**
 * Sets the span directly after this span.
 * May be null.
 */
void
ALMSpan::SetNext(ALMSpan* span)
{
	// If there should be no span directly after this span, then we have to
	// separate any such span and can remove any constraint that was used
	// to glue this span to it...
	if (span == NULL) {
		if (fNext == NULL)
			return;
		fNext->fPrevious = NULL;
		fNext->fPreviousGlue = NULL;
		fNext = NULL;
		delete fNextGlue;
		fNextGlue = NULL;
		return;
	}

	// ...otherwise we have to set up the pointers and the glue constraint
	// accordingly.
	if (span->fPrevious != NULL)
		span->SetPrevious(NULL);
	if (fNext != NULL)
		SetNext(NULL);

	fNext = span;
	fNext->fPrevious = this;
	span->fPreviousGlue = fEndTab->IsEqual(span->fStartTab);
	fNextGlue = span->fPreviousGlue;
}


/**
 * Inserts the given span directly above this span.
 *
 * @param span	the span to insert
 */
void
ALMSpan::InsertBefore(ALMSpan* span)
{
	SetPrevious(span->fPrevious);
	SetNext(span);
}


/**
 * Inserts the given span directly below this span.
 *
 * @param span	the span to insert
 */
void
ALMSpan::InsertAfter(ALMSpan* span)
{
	SetNext(span->fNext);
	SetPrevious(span);
}


/**
 * Constrains this span to have the same height as the given span.
 *
 * @param span	the span that should have the same height
 * @return the resulting same-height constraint
 */
Constraint*
ALMSpan::SetSameSizeAs(ALMSpan* span)
{
	Constraint* constraint = fLayout->AddConstraint(
		-1.0, fStartTab, 1.0, fEndTab, 1.0, span->fStartTab, -1.0,
		span->fEndTab, OperatorType(EQ), 0.0);
	fConstraints->AddItem(constraint);
	return constraint;
}


/**
 * Gets the constraints.
 */
BList*
ALMSpan::Constraints() const
{
	return fConstraints;
}


/**
 * Sets the constraints.
 */
void
ALMSpan::SetConstraints(BList* constraints)
{
	// TODO: What about previous fConstraints?
	fConstraints = constraints;
}


/**
 * Destructor.
 * Removes the span from the specification.
 */
ALMSpan::~ALMSpan()
{
	if (fPrevious != NULL)
		fPrevious->SetNext(fNext);
	for (int32 i = 0; i < fConstraints->CountItems(); i++)
		delete (Constraint*)fConstraints->ItemAt(i);
	delete fStartTab;
	delete fEndTab;
	// TODO: What about fConstraints?
}


/**
 * Constructor.
 */
ALMSpan::ALMSpan(ALMLayout* ls)
{
	fLayout = ls;
	fStartTab = new ALMTab(ls);
	fEndTab = new ALMTab(ls);
	fConstraints = new BList(1);
}

