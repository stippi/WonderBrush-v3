/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#ifndef OBJECT_ADDED_EDIT_H
#define OBJECT_ADDED_EDIT_H

#include "Layer.h"
#include "UndoableEdit.h"
#include "Selection.h"

class ObjectAddedEdit : public UndoableEdit, public Selection::Controller {
public:
	ObjectAddedEdit(Object* object, Selection* selection)
		: UndoableEdit()
		, fObject(object)

		, fInsertionLayer(NULL)
		, fInsertionIndex(-1)

		, fSelection(selection)
	{
		if (fObject == NULL)
			return;

		fObject->AddReference();

		fInsertionLayer = fObject->Parent();
		if (fInsertionLayer != NULL) {
			fInsertionLayer->AddReference();
			fInsertionIndex = fInsertionLayer->IndexOf(fObject);
		}
	}

	virtual ~ObjectAddedEdit()
	{
		if (fInsertionLayer != NULL)
			fInsertionLayer->RemoveReference();
		if (fObject != NULL)
			fObject->RemoveReference();
	}

	virtual status_t InitCheck()
	{
		if (fObject == NULL || fInsertionLayer == NULL)
			return B_NO_INIT;

		return B_OK;
	}

	virtual status_t Perform(EditContext& context)
	{
		// Object already added, but make sure it's selected as well.
		if (fSelection != NULL)
			fSelection->Select(Selectable(fObject), this, true);

		return B_OK;
	}

	virtual status_t Undo(EditContext& context)
	{
		fInsertionLayer->SuspendUpdates(true);

		if (fSelection != NULL)
			fSelection->DeselectAll(this);

		fInsertionLayer->RemoveObject(fObject);

		fInsertionLayer->SuspendUpdates(false);

		return B_OK;
	}

	virtual status_t Redo(EditContext& context)
	{
		fInsertionLayer->SuspendUpdates(true);

		if (fSelection != NULL)
			fSelection->DeselectAll(this);

		if (!fInsertionLayer->AddObject(fObject, fInsertionIndex)) {
			fInsertionLayer->SuspendUpdates(false);
			return B_NO_MEMORY;
		}
		if (fSelection != NULL)
			fSelection->Select(Selectable(fObject), this, true);

		fInsertionLayer->SuspendUpdates(false);

		return B_OK;
	}

	virtual void GetName(BString& name)
	{
		name << "Add object";
	}

private:
			Object*				fObject;

			Layer*				fInsertionLayer;
			int32				fInsertionIndex;

			Selection*			fSelection;
};

#endif // OBJECT_ADDED_EDIT_H
