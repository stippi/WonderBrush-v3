/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#include "Object.h"

#include "Layer.h"


// constructor
Object::Object()
	: fChangeCounter(0)
	, fParent(NULL)
{
}

// destructor
Object::~Object()
{
}

// #pragma mark -

// SetParent
void
Object::SetParent(Layer* layer)
{
	fParent = layer;
}

// UpdateChangeCounter
void
Object::UpdateChangeCounter()
{
	if (fParent)
		fParent->UpdateChangeCounter();
	fChangeCounter++;
}

