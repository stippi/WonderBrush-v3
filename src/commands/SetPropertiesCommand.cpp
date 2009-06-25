/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "SetPropertiesCommand.h"

#include <stdio.h>

#include "CommonPropertyIDs.h"
#include "BaseObject.h"
#include "Property.h"
#include "PropertyObject.h"

// constructor
SetPropertiesCommand::SetPropertiesCommand(BaseObject** objects,
		int32 objectCount, PropertyObject* previous, PropertyObject* current)
	:
	Command(),
	fObjects(objects),
	fObjectCount(objectCount),

	fOldProperties(previous),
	fNewProperties(current)
{
}

// destructor
SetPropertiesCommand::~SetPropertiesCommand()
{
	delete[] fObjects;
	delete fOldProperties;
	delete fNewProperties;
}

// InitCheck
status_t
SetPropertiesCommand::InitCheck()
{
	return fObjects && fOldProperties && fNewProperties
		   && fObjectCount > 0 && fOldProperties->CountProperties() > 0
		   && fOldProperties->ContainsSameProperties(*fNewProperties) ?
		   B_OK : B_NO_INIT;
}

// Perform
status_t
SetPropertiesCommand::Perform()
{
	for (int32 i = 0; i < fObjectCount; i++) {
		if (fObjects[i])
			fObjects[i]->SetToPropertyObject(fNewProperties);
	}
	return B_OK;
}

// Undo
status_t
SetPropertiesCommand::Undo()
{
	for (int32 i = 0; i < fObjectCount; i++) {
		if (fObjects[i])
			fObjects[i]->SetToPropertyObject(fOldProperties);
	}
	return B_OK;
}

// GetName
void
SetPropertiesCommand::GetName(BString& name)
{
	if (fOldProperties->CountProperties() > 1) {
		if (fObjectCount > 1)
			name << "Multi Paste Properties";
		else
			name << "Paste Properties";
	} else {
		BString property = name_for_id(
			fOldProperties->PropertyAt(0)->Identifier());
		if (fObjectCount > 1)
			name << "Multi Set " << property;
		else
			name << "Set " << property;
	}
}
