/*
 * Copyright 2007, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#ifndef CHANGE_AREA_ACTION_H
#define CHANGE_AREA_ACTION_H

#include <Rect.h>

#include "Command.h"
#include "Layer.h"

template<class ObjectType>
class ChangeAreaCommand : public Command {
 public:
	ChangeAreaCommand(Layer* layer, ObjectType* object, BRect newArea)
		: Command()
		, fLayer(layer)
		, fObject(object)
		, fNewArea(newArea)
	{
	}
	
	virtual ~ChangeAreaCommand()
	{
	}

	virtual	status_t InitCheck()
	{
		return fLayer && fObject && fNewArea != fObject->Area() ? B_OK : B_ERROR;
	}

	virtual	status_t Perform()
	{
		BRect oldArea = fObject->Area();
		fObject->SetArea(fNewArea);
		fLayer->Invalidate(oldArea | fNewArea, fLayer->IndexOf(fObject));
		fNewArea = oldArea;
	
		return B_OK;
	}

	virtual status_t Undo()
	{
		return Perform();
	}

	virtual void GetName(BString& name)
	{
		name << "Change Object Area";
	}

	virtual	bool CombineWithNext(const Command* _next)
	{
		const ChangeAreaCommand* next
			= dynamic_cast<const ChangeAreaCommand*>(_next);
	
		if (!next || next->fObject != fObject
			|| next->fTimeStamp - fTimeStamp > 500000)
			return false;
	
		fTimeStamp = next->fTimeStamp;
	
		return true;
	}

 private:
			Layer*				fLayer;
			ObjectType*			fObject;
			BRect				fNewArea;
};

#endif // CHANGE_AREA_ACTION_H
