/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef RECTANGLE_RADIUS_EDIT_H
#define RECTANGLE_RADIUS_EDIT_H

#include <String.h>

#include "ObjectEdit.h"
#include "Rect.h"

class RectangleRadiusEdit : public ObjectEdit<Rect> {
public:
	RectangleRadiusEdit(Rect* rectangle, double radius, Selection* selection)
		: ObjectEdit(rectangle, selection)
		, fRadius(radius)
	{
	}

	virtual ~RectangleRadiusEdit()
	{
	}

	virtual	status_t InitCheck()
	{
		return fObject.Get() != NULL && fObject->RoundCornerRadius() != fRadius
			? B_OK : B_ERROR;
	}

	virtual	status_t Perform(EditContext& context)
	{
		SelectObject();

		double radius = fObject->RoundCornerRadius();
		fObject->SetRoundCornerRadius(fRadius);
		fRadius = radius;

		return B_OK;
	}

	virtual status_t Undo(EditContext& context)
	{
		return Perform(context);
	}

	virtual	bool CombineWithNext(const UndoableEdit* _next)
	{
		const RectangleRadiusEdit* next
			= dynamic_cast<const RectangleRadiusEdit*>(_next);

		if (next == NULL || next->fObject != fObject
			|| next->fTimeStamp - fTimeStamp > 500000) {
			return false;
		}

		fTimeStamp = next->fTimeStamp;

		return true;
	}

	virtual void GetName(BString& name)
	{
		name << "Set rectangle corner radius";
	}

private:
			double			fRadius;
};

#endif // RECTANGLE_RADIUS_EDIT_H
