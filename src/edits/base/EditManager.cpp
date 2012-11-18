/*
 * Copyright 2006-2012, Stephan Aßmus <superstippi@gmx.de>
 * Distributed under the terms of the MIT License.
 */

#include "EditManager.h"

#include <stdio.h>
#include <string.h>

#include <Locker.h>
#include <String.h>

#include "RWLocker.h"

// constructor
EditManager::EditManager(RWLocker* locker)
	: Notifier()
	, fLocker(locker)
	, fEditAtSave()
{
}

// destructor
EditManager::~EditManager()
{
	Clear();
}

// Perform
status_t
EditManager::Perform(UndoableEdit* edit)
{
	if (edit == NULL)
		return B_BAD_VALUE;

	return Perform(UndoableEditRef(edit, true));
}

// Perform
status_t
EditManager::Perform(const UndoableEditRef& edit)
{
	if (!fLocker->WriteLock())
		return B_ERROR;

	status_t ret = edit.Get() != NULL ? B_OK : B_BAD_VALUE;
	if (ret == B_OK)
		ret = edit->InitCheck();

	if (ret == B_OK)
		ret = edit->Perform();

	if (ret == B_OK) {
		ret = _AddEdit(edit);
		if (ret != B_OK)
			edit->Undo();
	}

	fLocker->WriteUnlock();

	Notify();

	return ret;
}

// Undo
status_t
EditManager::Undo()
{
	if (!fLocker->WriteLock())
		return B_ERROR;

	status_t status = B_ERROR;
	if (!fUndoHistory.IsEmpty()) {
		UndoableEditRef edit(fUndoHistory.Top());
		fUndoHistory.Pop();
		status = edit->Undo();
		if (status == B_OK)
			fRedoHistory.Push(edit);
		else
			fUndoHistory.Push(edit);
	}
	fLocker->WriteUnlock();

	Notify();

	return status;
}

// Redo
status_t
EditManager::Redo()
{
	if (!fLocker->WriteLock())
		return B_ERROR;

	status_t status = B_ERROR;
	if (!fRedoHistory.IsEmpty()) {
		UndoableEditRef edit(fRedoHistory.Top());
		fRedoHistory.Pop();
		status = edit->Redo();
		if (status == B_OK)
			fUndoHistory.Push(edit);
		else
			fRedoHistory.Push(edit);
	}
	fLocker->WriteUnlock();

	Notify();

	return status;
}

// UndoName
bool
EditManager::GetUndoName(BString& name)
{
	bool success = false;
	if (fLocker->ReadLock()) {
		if (!fUndoHistory.IsEmpty()) {
			name << " ";
			fUndoHistory.Top()->GetName(name);
			success = true;
		}
		fLocker->ReadUnlock();
	}
	return success;
}

// RedoName
bool
EditManager::GetRedoName(BString& name)
{
	bool success = false;
	if (fLocker->ReadLock()) {
		if (!fRedoHistory.IsEmpty()) {
			name << " ";
			fRedoHistory.Top()->GetName(name);
			success = true;
		}
		fLocker->ReadUnlock();
	}
	return success;
}

// Clear
void
EditManager::Clear()
{
	if (fLocker->WriteLock()) {
		while (!fUndoHistory.IsEmpty())
			fUndoHistory.Pop();
		while (!fRedoHistory.IsEmpty())
			fRedoHistory.Pop();
		fLocker->WriteUnlock();
	}

	Notify();
}

// Save
void
EditManager::Save()
{
	if (fLocker->WriteLock()) {
		if (!fUndoHistory.IsEmpty())
			fEditAtSave = fUndoHistory.Top();
		fLocker->WriteUnlock();
	}

	Notify();
}

// IsSaved
bool
EditManager::IsSaved()
{
	bool saved = false;
	if (fLocker->ReadLock()) {
		saved = fUndoHistory.IsEmpty();
		if (fEditAtSave.Get() != NULL && !saved) {
			if (fEditAtSave == fUndoHistory.Top())
				saved = true;
		}
		fLocker->ReadUnlock();
	}
	return saved;
}

// #pragma mark -

// _AddEdit
status_t
EditManager::_AddEdit(const UndoableEditRef& edit)
{
	status_t status = B_OK;

	bool add = true;
	if (!fUndoHistory.IsEmpty()) {
		// try to collapse edits to a single edit
		// or remove this and the previous edit if
		// they reverse each other
		const UndoableEditRef& top = fUndoHistory.Top();
		if (edit->UndoesPrevious(top.Get())) {
			add = false;
			fUndoHistory.Pop();
		} else if (top->CombineWithNext(edit.Get())) {
			add = false;
			// after collapsing, the edit might
			// have changed it's mind about InitCheck()
			// (the commands reversed each other)
			if (top->InitCheck() != B_OK) {
				fUndoHistory.Pop();
			}
		} else if (edit->CombineWithPrevious(top.Get())) {
			fUndoHistory.Pop();
			// after collapsing, the edit might
			// have changed it's mind about InitCheck()
			// (the commands reversed each other)
			if (edit->InitCheck() != B_OK) {
				add = false;
			}
		}
	}
	if (add) {
		if (!fUndoHistory.Push(edit))
			status = B_NO_MEMORY;
	}

	if (status == B_OK) {
		// the redo stack needs to be empty
		// as soon as a edit was added (also in case of collapsing)
		while (!fRedoHistory.IsEmpty()) {
			fRedoHistory.Pop();
		}
	}

	return status;
}


