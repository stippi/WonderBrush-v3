/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef OBJECT_EDIT_H
#define OBJECT_EDIT_H

#include <String.h>

#include "Selection.h"
#include "Object.h"
#include "UndoableEdit.h"

template<class ObjectType>
class ObjectEdit : public UndoableEdit, public Selection::Controller {
public:
	ObjectEdit(ObjectType* object, Selection* selection)
		: UndoableEdit()
		, fObject(object)
		, fSelection(selection)
	{
	}

	virtual ~ObjectEdit()
	{
	}

protected:
	void SelectObject()
	{
		if (fSelection == NULL || fObject.Get() == NULL)
			return;

		fSelection->Select(Selectable(fObject.Get()), this, false);
	}

protected:
			Reference<ObjectType>	fObject;
			Selection*				fSelection;
};

#endif // OBJECT_EDIT_H
