/*
 * Copyright 2006-2112, Stephan Aßmus <superstippi@gmx.de>
 * Distributed under the terms of the MIT License.
 */

#include "CompoundEdit.h"

#include <stdio.h>

// constructor
CompoundEdit::CompoundEdit(const char* name)
	: UndoableEdit()
	, fEdits()
	, fName(name)
{
}

// destructor
CompoundEdit::~CompoundEdit()
{
}

// InitCheck
status_t
CompoundEdit::InitCheck()
{
	return B_OK;
}

// Perform
status_t
CompoundEdit::Perform(EditContext& context)
{
	status_t status = B_OK;

	int32 count = fEdits.CountItems();
	int32 i = 0;
	for (; i < count; i++) {
		status = fEdits.ItemAtFast(i)->Perform(context);
		if (status != B_OK)
			break;
	}

	if (status != B_OK) {
		// roll back
		i--;
		for (; i >= 0; i--) {
			fEdits.ItemAtFast(i)->Undo(context);
		}
	}

	return status;
}

// Undo
status_t
CompoundEdit::Undo(EditContext& context)
{
	status_t status = B_OK;

	int32 count = fEdits.CountItems();
	int32 i = count - 1;
	for (; i >= 0; i--) {
		status = fEdits.ItemAtFast(i)->Undo(context);
		if (status != B_OK)
			break;
	}

	if (status != B_OK) {
		// roll back
		i++;
		for (; i < count; i++) {
			fEdits.ItemAtFast(i)->Redo(context);
		}
	}

	return status;
}

// Redo
status_t
CompoundEdit::Redo(EditContext& context)
{
	status_t status = B_OK;

	int32 count = fEdits.CountItems();
	int32 i = 0;
	for (; i < count; i++) {
		status = fEdits.ItemAtFast(i)->Redo(context);
		if (status != B_OK)
			break;
	}

	if (status != B_OK) {
		// roll back
		i--;
		for (; i >= 0; i--) {
			fEdits.ItemAtFast(i)->Undo(context);
		}
	}

	return status;
}

// GetName
void
CompoundEdit::GetName(BString& name)
{
	name << fName;
}

// AppendEdit
bool
CompoundEdit::AppendEdit(const UndoableEditRef& edit)
{
	return fEdits.Add(edit);
}
