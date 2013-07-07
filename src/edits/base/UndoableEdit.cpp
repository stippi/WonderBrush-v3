/*
 * Copyright 2006-2012, Stephan Aßmus <superstippi@gmx.de>
 * Distributed under the terms of the MIT License.
 */

#include "UndoableEdit.h"

#include <stdio.h>

#include <OS.h>
#include <String.h>

//static int32 sInstanceCount = 0;

// constructor
UndoableEdit::UndoableEdit()
	:
	fTimeStamp(system_time())
{
//	sInstanceCount++;
//	printf("UndoableEdits: %ld\n", sInstanceCount);
}

// destructor
UndoableEdit::~UndoableEdit()
{
//	sInstanceCount--;
//	printf("UndoableEdits: %ld\n", sInstanceCount);
}

// InitCheck
status_t
UndoableEdit::InitCheck()
{
	return B_NO_INIT;
}

// Perform
status_t
UndoableEdit::Perform(EditContext& context)
{
	return B_ERROR;
}

// Undo
status_t
UndoableEdit::Undo(EditContext& context)
{
	return B_ERROR;
}

// Redo
status_t
UndoableEdit::Redo(EditContext& context)
{
	return Perform(context);
}

// GetName
void
UndoableEdit::GetName(BString& name)
{
	name << "Name of edit goes here.";
}

// UndoesPrevious
bool
UndoableEdit::UndoesPrevious(const UndoableEdit* previous)
{
	return false;
}

// CombineWithNext
bool
UndoableEdit::CombineWithNext(const UndoableEdit* next)
{
	return false;
}

// CombineWithPrevious
bool
UndoableEdit::CombineWithPrevious(const UndoableEdit* previous)
{
	return false;
}
