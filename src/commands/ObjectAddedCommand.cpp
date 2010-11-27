/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "ObjectAddedCommand.h"

#include <new>

#include <stdio.h>

#include "Layer.h"

// constructor
ObjectAddedCommand::ObjectAddedCommand(Object* object, Selection* selection)
	: Command()
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

// destructor
ObjectAddedCommand::~ObjectAddedCommand()
{
	if (fInsertionLayer != NULL)
		fInsertionLayer->RemoveReference();
	if (fObject != NULL)
		fObject->RemoveReference();
}

// InitCheck
status_t
ObjectAddedCommand::InitCheck()
{
	if (fObject == NULL || fInsertionLayer == NULL)
		return B_NO_INIT;

	return B_OK;
}

// Perform
status_t
ObjectAddedCommand::Perform()
{
	// Object already added, but make sure it's selected as well.
	if (fSelection != NULL)
		fSelection->Select(Selectable(fObject), this, true);

	return B_OK;
}

// Undo
status_t
ObjectAddedCommand::Undo()
{
	fInsertionLayer->SuspendUpdates(true);

	if (fSelection != NULL)
		fSelection->DeselectAll(this);

	fInsertionLayer->RemoveObject(fObject);

	fInsertionLayer->SuspendUpdates(false);

	return B_OK;
}

// Redo
status_t
ObjectAddedCommand::Redo()
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

// GetName
void
ObjectAddedCommand::GetName(BString& name)
{
	name << "Add object";
}

