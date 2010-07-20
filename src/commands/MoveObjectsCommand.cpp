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
		int32 objectCount, Layer* insertionLayer, int32 insertionIndex,
		Selection* selection)
	: Command()
	, fObjects(objects)
	, fObjectCount(objectCount)

	, fOldPositions(new(std::nothrow) PositionInfo[objectCount])

	, fInsertionLayer(insertionLayer)
	, fInsertionIndex(insertionIndex)

	, fSelection(selection)
{
//printf("MoveObjectsCommand::MoveObjectsCommand(%ld, %p, %ld)\n",
//	objectCount, insertionLayer, insertionIndex);

	if (fObjects == NULL || fObjectCount <= 0 || fOldPositions == NULL)
		return;

	// Remember current positions of all objects.
	for (int32 i = 0; i < fObjectCount; i++) {
		// Check if this object is a distant child of any of the other
		// objects that we are moving and remove it from the array in that
		// case.
		bool ignoreObject = false;
		for (int32 j = 0; j < fObjectCount; j++) {
			if (j == i)
				continue;
			Layer* layer = dynamic_cast<Layer*>(fObjects[j]);
			if (layer == NULL)
				continue;
			if (_ObjectIsDistantChildOf(fObjects[i], layer)) {
//printf("ignoring %p\n", fObjects[i]);
				ignoreObject = true;
				break;
			}
		}
		if (ignoreObject) {
			fObjects[i] = NULL;
			continue;
		}

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
	if (fObjects == NULL || fOldPositions == NULL || fObjectCount <= 0
		|| fInsertionLayer == NULL) {
		return B_NO_INIT;
	}

	// Check if object tree changes.
	PositionInfo newPositions[fObjectCount];
	int32 index = fInsertionIndex;
	for (int32 i = 0; i < fObjectCount; i++) {
		newPositions[i].parent = fInsertionLayer;
		newPositions[i].index = index++;
	}
	if (memcmp(newPositions, fOldPositions, sizeof(newPositions)) == 0) {
//printf("MoveObjectsCommand::InitCheck(): no changes!\n");
		return B_BAD_VALUE;
	}

	// Check that we are not trying to add a layer to one of it's sub-layers
	// or to itself.
	for (int32 i = 0; i < fObjectCount; i++) {
		if (fObjects[i] == fInsertionLayer) {
//printf("MoveObjectsCommand::InitCheck(): Cannot add layer to itself!\n");
			return B_BAD_VALUE;
		}
		Layer* layer = dynamic_cast<Layer*>(fObjects[i]);
		if (layer == NULL)
			continue;
		if (_ObjectIsDistantChildOf(fInsertionLayer, layer)) {
//printf("MoveObjectsCommand::InitCheck(): Cannot add layer to one of it's "
//	"sub-layers!\n");
			return B_BAD_VALUE;
		}
	}

	return B_OK;
}

// Perform
status_t
MoveObjectsCommand::Perform()
{
	fInsertionLayer->SuspendUpdates(true);

	fSelection->DeselectAll(this);

	for (int32 i = 0; i < fObjectCount; i++) {
		if (fObjects[i] == NULL)
			continue;
		if (fOldPositions[i].parent != NULL)
			fOldPositions[i].parent->RemoveObject(fObjects[i]);
	}
	int32 index = fInsertionIndex;
	for (int32 i = 0; i < fObjectCount; i++) {
		if (fObjects[i] == NULL)
			continue;
		fInsertionLayer->AddObject(fObjects[i], index);
		index++;
		fSelection->Select(Selectable(fObjects[i]), this, true);
	}

	fInsertionLayer->SuspendUpdates(false);

	return B_OK;
}

// Undo
status_t
MoveObjectsCommand::Undo()
{
	fInsertionLayer->SuspendUpdates(true);

	fSelection->DeselectAll(this);

	for (int32 i = 0; i < fObjectCount; i++) {
		if (fObjects[i] == NULL)
			continue;
		fInsertionLayer->RemoveObject(fObjects[i]);
	}
	for (int32 i = 0; i < fObjectCount; i++) {
		if (fObjects[i] == NULL)
			continue;
		if (fOldPositions[i].parent != NULL) {
			fOldPositions[i].parent->AddObject(fObjects[i],
				fOldPositions[i].index);
		}
		fSelection->Select(Selectable(fObjects[i]), this, true);
	}

	fInsertionLayer->SuspendUpdates(false);

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

// #pragma mark -

// _ObjectIsDistantChildOf
bool
MoveObjectsCommand::_ObjectIsDistantChildOf(const Object* object,
	const Layer* layer) const
{
	int32 count = layer->CountObjects();
	for (int32 i = 0; i < count; i++) {
		const Object* child = layer->ObjectAtFast(i);
		if (object == child)
			return true;
		const Layer* subLayer = dynamic_cast<const Layer*>(child);
		if (subLayer != NULL && _ObjectIsDistantChildOf(object, subLayer))
			return true;
			
	}
	return false;
}
