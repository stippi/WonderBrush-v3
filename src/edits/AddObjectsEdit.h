/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#ifndef ADD_OBJECTS_EDIT_H
#define ADD_OBJECTS_EDIT_H

#include "Selection.h"
#include "UndoableEdit.h"

class Layer;
class Object;

class AddObjectsEdit : public UndoableEdit, public Selection::Controller {
public:
	AddObjectsEdit(Object** objects, int32 objectCount, Layer* insertionLayer,
		int32 insertionIndex, Selection* selection)
		: UndoableEdit()
		, fObjects(objects)
		, fObjectCount(objectCount)

		, fInsertionLayer(insertionLayer)
		, fInsertionIndex(insertionIndex)

		, fSelection(selection)
	{
		if (fInsertionLayer != NULL)
			fInsertionLayer->AddReference();

		if (fObjects == NULL || fObjectCount <= 0)
			return;

		// Remember current positions of all objects.
		for (int32 i = 0; i < fObjectCount; i++)
			fObjects[i]->AddReference();
	}

	virtual ~AddObjectsEdit()
	{
		if (fObjects != NULL) {
			for (int32 i = 0; i < fObjectCount; i++)
				fObjects[i]->RemoveReference();
			delete[] fObjects;
		}
		if (fInsertionLayer != NULL)
			fInsertionLayer->RemoveReference();
	}

	virtual status_t InitCheck()
	{
		if (fObjects == NULL || fObjectCount <= 0 || fInsertionLayer == NULL)
			return B_NO_INIT;

		return B_OK;
	}

	virtual status_t Perform(EditContext& context)
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

	virtual status_t Undo(EditContext& context)
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

	virtual void GetName(BString& name)
	{
		if (fObjectCount > 1)
			name << "Add objects";
		else
			name << "Add object";
	}

private:
			Object**			fObjects;
			int32				fObjectCount;

			Layer*				fInsertionLayer;
			int32				fInsertionIndex;

			Selection*			fSelection;
};

#endif // ADD_OBJECTS_EDIT_H
