/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "MoveObjectsCommand.h"

#include <new>

#include <stdio.h>

#include "Layer.h"

// constructor
MoveObjectsCommand::MoveObjectsCommand(Object** objects,
		int32 objectCount, Layer* insertionLayer, int32 insertionIndex)
	: Command()
	, fObjects(objects)
	, fObjectCount(objectCount)

	, fOldPositions(new(std::nothrow) PositionInfo[objectCount])

	, fInsertionLayer(insertionLayer)
	, fInsertionIndex(insertionIndex)
{
printf("MoveObjectsCommand::MoveObjectsCommand(%ld, %p, %ld)\n",
	objectCount, insertionLayer, insertionIndex);

	if (fObjects == NULL || fObjectCount <= 0 || fOldPositions == NULL)
		return;

	// Remember current positions of all objects.
	for (int32 i = 0; i < fObjectCount; i++) {
		fObjects[i]->AddReference();

		Layer* parent = fObjects[i]->Parent();
		fOldPositions[i].parent = parent;
		if (parent != NULL)
			fOldPositions[i].index = parent->IndexOf(fObjects[i]);
		else
			fOldPositions[i].index = 0;
		if (parent == fInsertionLayer
			&& fOldPositions[i].index < fInsertionIndex) {
			fInsertionIndex--;
		}
	}
}

// destructor
MoveObjectsCommand::~MoveObjectsCommand()
{
	if (fObjects != NULL) {
		for (int32 i = 0; i < fObjectCount; i++)
			fObjects[i]->RemoveReference();
		delete[] fObjects;
	}
	delete[] fOldPositions;
}

// InitCheck
status_t
MoveObjectsCommand::InitCheck()
{
	// TODO: Check if object tree changes!
	return fObjects && fOldPositions && fObjectCount > 0 && fInsertionLayer
		? B_OK : B_NO_INIT;
}

// Perform
status_t
MoveObjectsCommand::Perform()
{
	for (int32 i = 0; i < fObjectCount; i++) {
		if (fOldPositions[i].parent != NULL)
			fOldPositions[i].parent->RemoveObject(fObjects[i]);
	}
	int32 index = fInsertionIndex;
	for (int32 i = 0; i < fObjectCount; i++) {
		fInsertionLayer->AddObject(fObjects[i], index);
		index++;
	}
	return B_OK;
}

// Undo
status_t
MoveObjectsCommand::Undo()
{
	for (int32 i = 0; i < fObjectCount; i++) {
		fInsertionLayer->RemoveObject(fObjects[i]);
	}
	for (int32 i = fObjectCount - 1; i >= 0; i--) {
		if (fOldPositions[i].parent != NULL) {
			fOldPositions[i].parent->AddObject(fObjects[i],
				fOldPositions[i].index);
		}
	}
	return B_OK;
}

// GetName
void
MoveObjectsCommand::GetName(BString& name)
{
	if (fObjectCount > 1)
		name << "Move Objects";
	else
		name << "Move Object";
}
