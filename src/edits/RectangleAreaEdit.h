/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef RECTANGLE_AREA_EDIT_H
#define RECTANGLE_AREA_EDIT_H

#include <String.h>

#include "ObjectEdit.h"
#include "Rect.h"

class RectangleAreaEdit : public ObjectEdit<Rect> {
public:
	RectangleAreaEdit(Rect* rectangle, const BRect& area, Selection* selection)
		: ObjectEdit(rectangle, selection)
		, fArea(area)
	{
	}

	virtual ~RectangleAreaEdit()
	{
	}

	virtual	status_t InitCheck()
	{
		return fObject.Get() != NULL && fObject->Area() != fArea
			? B_OK : B_ERROR;
	}

	virtual	status_t Perform(EditContext& context)
	{
		SelectObject();

		BRect area = fObject->Area();
		fObject->SetArea(fArea);
		fArea = area;

		return B_OK;
	}

	virtual status_t Undo(EditContext& context)
	{
		return Perform(context);
	}

	virtual	bool CombineWithNext(const UndoableEdit* _next)
	{
		const RectangleAreaEdit* next
			= dynamic_cast<const RectangleAreaEdit*>(_next);

		if (next == NULL || next->fObject != fObject
			|| next->fTimeStamp - fTimeStamp > 500000) {
			return false;
		}

		fTimeStamp = next->fTimeStamp;

		return true;
	}

	virtual void GetName(BString& name)
	{
		name << "Edit rectangle";
	}

private:
			BRect			fArea;
};

#endif // RECTANGLE_AREA_EDIT_H
