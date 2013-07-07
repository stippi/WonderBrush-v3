/*
 * Copyright 2010-2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "RemoveObjectsEdit.h"

#include <new>

#include <stdio.h>

#include "Layer.h"

// constructor
RemoveObjectsEdit::RemoveObjectsEdit(const ObjectList& objects,
		Selection* selection)
	: UndoableEdit()
	, fObjects(objects)

	, fOldPositions(new(std::nothrow) PositionInfo[objects.CountItems()])

	, fSelection(selection)
{
//printf("RemoveObjectsEdit::RemoveObjectsEdit(%ld, %p, %ld)\n",
//	objectCount, insertionLayer, insertionIndex);
	if (fObjects.CountItems() == 0 || fOldPositions == NULL)
		return;

	// Remember current positions of all objects.
	for (int32 i = 0; i < fObjects.CountItems(); i++) {
		// Check if this object is a distant child of any of the other
		// objects that we are moving and remove it from the array in that
		// case.
		bool ignoreObject = false;
		for (int32 j = fObjects.CountItems() - 1; j >= 0; j--) {
			if (j == i)
				continue;
			Layer* layer = dynamic_cast<Layer*>(fObjects.ItemAt(j).Get());
			if (layer == NULL)
				continue;
			if (_ObjectIsDistantChildOf(fObjects.ItemAt(i).Get(), layer)) {
//printf("ignoring %p\n", fObjects[i]);
				ignoreObject = true;
				break;
			}
		}
		if (ignoreObject) {
			fOldPositions[i].parent = NULL;
		} else {
			Layer* parent = fObjects.ItemAt(i)->Parent();
			fOldPositions[i].parent = parent;
			if (parent != NULL) {
				fOldPositions[i].index = parent->IndexOf(
					fObjects.ItemAt(i).Get());
			} else {
				fOldPositions[i].index = 0;
			}
		}
	}
}

// destructor
RemoveObjectsEdit::~RemoveObjectsEdit()
{
	delete[] fOldPositions;
}

// InitCheck
status_t
RemoveObjectsEdit::InitCheck()
{
	if (fObjects.CountItems() == 0 || fOldPositions == NULL)
		return B_NO_INIT;

	return B_OK;
}

// Perform
status_t
RemoveObjectsEdit::Perform(EditContext& context)
{
	if (fSelection != NULL)
		fSelection->DeselectAll(this);

	for (int32 i = 0; i < fObjects.CountItems(); i++) {
		if (fOldPositions[i].parent != NULL)
			fOldPositions[i].parent->RemoveObject(fObjects.ItemAt(i).Get());
	}

	return B_OK;
}

// Undo
status_t
RemoveObjectsEdit::Undo(EditContext& context)
{
	if (fSelection != NULL)
		fSelection->DeselectAll(this);

	for (int32 i = 0; i < fObjects.CountItems(); i++) {
		Object* object = fObjects.ItemAt(i).Get();
		if (fOldPositions[i].parent != NULL) {
			fOldPositions[i].parent->AddObject(object,
				fOldPositions[i].index);
		}
		if (fSelection != NULL)
			fSelection->Select(Selectable(object), this, true);
	}

	return B_OK;
}

// GetName
void
RemoveObjectsEdit::GetName(BString& name)
{
	if (fObjects.CountItems() > 1)
		name << "Remove objects";
	else
		name << "Remove object";
}

// #pragma mark -

// _ObjectIsDistantChildOf
bool
RemoveObjectsEdit::_ObjectIsDistantChildOf(const Object* object,
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
