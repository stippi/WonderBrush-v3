/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>
 * Distributed under the terms of the MIT License.
 */

#include "EditStack.h"

#include <stdio.h>

// constructor
EditStack::EditStack()
	: fEdits()
{
}

// destructor
EditStack::~EditStack()
{
}

// Push
bool
EditStack::Push(const UndoableEditRef& edit)
{
	return fEdits.Add(edit);
}

// Pop
UndoableEditRef
EditStack::Pop()
{
	UndoableEditRef edit(Top());
	fEdits.Remove();
	return edit;
}

// Top
const UndoableEditRef&
EditStack::Top() const
{
	return fEdits.LastItem();
}

// IsEmpty
bool
EditStack::IsEmpty() const
{
	return fEdits.CountItems() == 0;
}
