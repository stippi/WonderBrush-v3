/*
 * Copyright 2006-2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef SET_PROPERTIES_EDIT_H
#define SET_PROPERTIES_EDIT_H

#include "UndoableEdit.h"

class BaseObject;
class PropertyObject;

class SetPropertiesEdit : public UndoableEdit {
public:
	SetPropertiesEdit(BaseObject** objects, int32 objectCount,
		PropertyObject* previous, PropertyObject* current)
		:
		UndoableEdit(),
		fObjects(objects),
		fObjectCount(objectCount),

		fOldProperties(previous),
		fNewProperties(current)
	{
	}

	virtual ~SetPropertiesEdit()
	{
		delete[] fObjects;
		delete fOldProperties;
		delete fNewProperties;
	}

	virtual status_t InitCheck()
	{
		return fObjects && fOldProperties && fNewProperties
			&& fObjectCount > 0 && fOldProperties->CountProperties() > 0
			&& fOldProperties->ContainsSameProperties(*fNewProperties)
			? B_OK : B_NO_INIT;
	}

	virtual status_t Perform(EditContext& context)
	{
		for (int32 i = 0; i < fObjectCount; i++) {
			if (fObjects[i])
				fObjects[i]->SetToPropertyObject(fNewProperties);
		}
		return B_OK;
	}

	virtual status_t Undo(EditContext& context)
	{
		for (int32 i = 0; i < fObjectCount; i++) {
			if (fObjects[i])
				fObjects[i]->SetToPropertyObject(fOldProperties);
		}
		return B_OK;
	}

	virtual void GetName(BString& name)
	{
		if (fOldProperties->CountProperties() > 1) {
			if (fObjectCount > 1)
				name << "Multi paste properties";
			else
				name << "Paste properties";
		} else {
			BString property = fOldProperties->PropertyAt(0)->Name();
			if (fObjectCount > 1)
				name << "Multi set " << property;
			else
				name << "Set " << property;
		}
	}

private:
	BaseObject**				fObjects;
	int32						fObjectCount;

	PropertyObject*				fOldProperties;
	PropertyObject*				fNewProperties;
};

#endif // SET_PROPERTIES_EDIT_H
