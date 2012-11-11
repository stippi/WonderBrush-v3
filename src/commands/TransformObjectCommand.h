/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef TRANSFORM_OBJECT_COMMAND_H
#define TRANSFORM_OBJECT_COMMAND_H

#include <Rect.h>

#include "Command.h"
#include "Object.h"

class TransformObjectCommand : public Command {
 public:
	TransformObjectCommand(Object* object,
		const Transformable& newTransformation)
		: Command()
		, fObject(object)
		, fTransformation(newTransformation)
	{
		fObject->AddReference();
	}
	
	virtual ~TransformObjectCommand()
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

	virtual	bool CombineWithNext(const Command* _next)
	{
		const TransformObjectCommand* next
			= dynamic_cast<const TransformObjectCommand*>(_next);
	
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

#endif // TRANSFORM_OBJECT_COMMAND_H
