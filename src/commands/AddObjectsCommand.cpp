/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "AddObjectsCommand.h"

#include <new>

#include <stdio.h>

#include "Layer.h"

// constructor
AddObjectsCommand::AddObjectsCommand(Object** objects,
		int32 objectCount, Layer* insertionLayer, int32 insertionIndex,
		Selection* selection)
	: Command()
	, fObjects(objects)
	, fObjectCount(objectCount)

	, fInsertionLayer(insertionLayer)
	, fInsertionIndex(insertionIndex)

	, fSelection(selection)
{
//printf("AddObjectsCommand::AddObjectsCommand(%ld, %p, %ld)\n",
//	objectCount, insertionLayer, insertionIndex);

	if (fInsertionLayer != NULL)
		fInsertionLayer->AddReference();

	if (fObjects == NULL || fObjectCount <= 0)
		return;

	// Remember current positions of all objects.
	for (int32 i = 0; i < fObjectCount; i++)
		fObjects[i]->AddReference();
}

// destructor
AddObjectsCommand::~AddObjectsCommand()
{
	if (fObjects != NULL) {
		for (int32 i = 0; i < fObjectCount; i++)
			fObjects[i]->RemoveReference();
		delete[] fObjects;
	}
	if (fInsertionLayer != NULL)
		fInsertionLayer->RemoveReference();
}

// InitCheck
status_t
AddObjectsCommand::InitCheck()
{
	if (fObjects == NULL || fObjectCount <= 0 || fInsertionLayer == NULL)
		return B_NO_INIT;

	return B_OK;
}

// Perform
status_t
AddObjectsCommand::Perform()
{
	fInsertionLayer->SuspendUpdates(true);

	if (fSelection != NULL)
		fSelection->DeselectAll(this);

	int32 index = fInsertionIndex;
	for (int32 i = 0; i < fObjectCount; i++) {
		if (fObjects[i] == NULL)
			continue;
		if (!fInsertionLayer->AddObject(fObjects[i], index)) {
			fInsertionLayer->SuspendUpdates(false);
			return B_NO_MEMORY;
		}
		index++;
		if (fSelection != NULL)
			fSelection->Select(Selectable(fObjects[i]), this, true);
	}

	fInsertionLayer->SuspendUpdates(false);

	return B_OK;
}

// Undo
status_t
AddObjectsCommand::Undo()
{
	fInsertionLayer->SuspendUpdates(true);

	if (fSelection != NULL)
		fSelection->DeselectAll(this);

	for (int32 i = 0; i < fObjectCount; i++) {
		if (fObjects[i] == NULL)
			continue;
		fInsertionLayer->RemoveObject(fObjects[i]);
	}

	fInsertionLayer->SuspendUpdates(false);

	return B_OK;
}

// GetName
void
AddObjectsCommand::GetName(BString& name)
{
	if (fObjectCount > 1)
		name << "Add objects";
	else
		name << "Add object";
}

