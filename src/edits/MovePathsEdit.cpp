/*
 * Copyright 2010-2018, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "MovePathsEdit.h"

#include <new>

#include <stdio.h>

#include "PathInstance.h"
#include "Shape.h"

// constructor
MovePathsEdit::MovePathsEdit(PathInstance** paths,
		int32 pathCount, Shape* insertionShape, int32 insertionIndex,
		Selection* selection)
	: UndoableEdit()
	, fPaths(paths)
	, fPathCount(pathCount)

	, fOldPositions(new(std::nothrow) PositionInfo[pathCount])

	, fInsertionShape(insertionShape)
	, fInsertionIndex(insertionIndex)

	, fSelection(selection)
{
//printf("MovePathsEdit::MovePathsEdit(%ld, %p, %ld)\n",
//	pathCount, insertionLayer, insertionIndex);

	if (fInsertionShape != NULL)
		fInsertionShape->AddReference();

	if (fPaths == NULL || fPathCount <= 0 || fOldPositions == NULL)
		return;

	// Remember current positions of all paths.
	for (int32 i = 0; i < fPathCount; i++) {
		if (fInsertionShape->ContainsPath(fPaths[i]->Path())) {
			fPaths[i] = NULL;
			fOldPositions[i].parent = NULL;
			fOldPositions[i].index = 0;
			continue;
		}
		
		fPaths[i]->AddReference();

		Shape* parent = fPaths[i]->Shape();
		fOldPositions[i].parent = parent;
		if (parent != NULL)
			fOldPositions[i].index = parent->Paths().IndexOf(fPaths[i]);
		else
			fOldPositions[i].index = 0;
		if (parent == fInsertionShape
			&& fOldPositions[i].index < fInsertionIndex) {
			fInsertionIndex--;
		}
	}
}

// destructor
MovePathsEdit::~MovePathsEdit()
{
	if (fPaths != NULL) {
		for (int32 i = 0; i < fPathCount; i++) {
			if (fPaths[i] != NULL)
				fPaths[i]->RemoveReference();
		}
		delete[] fPaths;
	}

	delete[] fOldPositions;

	if (fInsertionShape != NULL)
		fInsertionShape->RemoveReference();
}

// InitCheck
status_t
MovePathsEdit::InitCheck()
{
	if (fPaths == NULL || fOldPositions == NULL || fPathCount <= 0
		|| fInsertionShape == NULL) {
		return B_NO_INIT;
	}

	// Check if object tree changes.
	PositionInfo newPositions[fPathCount];
	int32 index = fInsertionIndex;
	for (int32 i = 0; i < fPathCount; i++) {
		if (fPaths[i] == NULL) {
			newPositions[i].parent = NULL;
			newPositions[i].index = index = 0;
		} else {
			newPositions[i].parent = fInsertionShape;
			newPositions[i].index = index++;
		}
	}
	if (memcmp(newPositions, fOldPositions, sizeof(newPositions)) == 0) {
//printf("MovePathsEdit::InitCheck(): no changes!\n");
		return B_BAD_VALUE;
	}

	return B_OK;
}

// Perform
status_t
MovePathsEdit::Perform(EditContext& context)
{
//	fInsertionShape->SuspendUpdates(true);

	if (fSelection != NULL)
		fSelection->DeselectAll(this);

	for (int32 i = 0; i < fPathCount; i++) {
		if (fPaths[i] == NULL)
			continue;
		if (fOldPositions[i].parent != NULL)
			fOldPositions[i].parent->RemovePath(fPaths[i]->Path());
	}
	int32 index = fInsertionIndex;
	for (int32 i = 0; i < fPathCount; i++) {
		if (fPaths[i] == NULL)
			continue;
		if (fInsertionShape->AddPath(fPaths[i]->Path(), index) == NULL) {
//			fInsertionShape->SuspendUpdates(false);
			return B_NO_MEMORY;
		}
		index++;
		if (fSelection != NULL)
			fSelection->Select(Selectable(fPaths[i]), this, true);
	}

//	fInsertionShape->SuspendUpdates(false);

	return B_OK;
}

// Undo
status_t
MovePathsEdit::Undo(EditContext& context)
{
//	fInsertionShape->SuspendUpdates(true);

	if (fSelection != NULL)
		fSelection->DeselectAll(this);

	for (int32 i = 0; i < fPathCount; i++) {
		if (fPaths[i] == NULL)
			continue;
		fInsertionShape->RemovePath(fPaths[i]->Path());
	}
	for (int32 i = 0; i < fPathCount; i++) {
		if (fPaths[i] == NULL)
			continue;
		if (fOldPositions[i].parent != NULL) {
			fOldPositions[i].parent->AddPath(fPaths[i]->Path(),
				fOldPositions[i].index);
		}
		if (fSelection != NULL)
			fSelection->Select(Selectable(fPaths[i]), this, true);
	}

//	fInsertionShape->SuspendUpdates(false);

	return B_OK;
}

// GetName
void
MovePathsEdit::GetName(BString& name)
{
	if (fPathCount > 1)
		name << "Move paths";
	else
		name << "Move path";
}

