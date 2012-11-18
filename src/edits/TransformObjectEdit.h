/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef TRANSFORM_OBJECT_EDIT_H
#define TRANSFORM_OBJECT_EDIT_H

#include <Rect.h>

#include "Object.h"
#include "UndoableEdit.h"

class TransformObjectEdit : public UndoableEdit {
 public:
	TransformObjectEdit(Object* object,
		const Transformable& newTransformation)
		: UndoableEdit()
		, fObject(object)
		, fTransformation(newTransformation)
	{
		fObject->AddReference();
	}

	virtual ~TransformObjectEdit()
	{
		fObject->RemoveReference();
	}

	virtual	status_t InitCheck()
	{
		return *fObject == fTransformation ? B_ERROR : B_OK;
	}

	virtual	status_t Perform()
	{
		Transformable previousTransformation = *fObject;
		fObject->SetTransformable(fTransformation);
		fTransformation = previousTransformation;

		return B_OK;
	}

	virtual status_t Undo()
	{
		return Perform();
	}

	virtual void GetName(BString& name)
	{
		name << "Change transformation";
	}

	virtual	bool CombineWithNext(const UndoableEdit* _next)
	{
		const TransformObjectEdit* next
			= dynamic_cast<const TransformObjectEdit*>(_next);

		if (next == NULL || next->fObject != fObject
			|| next->fTimeStamp - fTimeStamp > 500000) {
			return false;
		}

		fTimeStamp = next->fTimeStamp;

		return true;
	}

 private:
			Object*				fObject;
			Transformable		fTransformation;
};

#endif // TRANSFORM_OBJECT_EDIT_H
