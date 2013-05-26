/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef SET_TEXT_WIDTH_EDIT_H
#define SET_TEXT_WIDTH_EDIT_H

#include <String.h>

#include "UndoableEdit.h"
#include "Text.h"

class SetTextWidthEdit : public UndoableEdit {
public:
	SetTextWidthEdit(Text* text, double width)
		: UndoableEdit()
		, fText(text)
		, fWidth(width)
	{
	}

	virtual ~SetTextWidthEdit()
	{
	}

	virtual	status_t InitCheck()
	{
		return fText->Width() != fWidth ? B_OK : B_ERROR;
	}

	virtual	status_t Perform()
	{
		double previousWidth = fText->Width();
		fText->SetWidth(fWidth);
		fWidth = previousWidth;

		return B_OK;
	}

	virtual status_t Undo()
	{
		return Perform();
	}

	virtual void GetName(BString& name)
	{
		name << "Change text width";
	}

	virtual	bool CombineWithNext(const UndoableEdit* _next)
	{
		const SetTextWidthEdit* next
			= dynamic_cast<const SetTextWidthEdit*>(_next);

		if (next == NULL || next->fText != fText
			|| next->fTimeStamp - fTimeStamp > 500000) {
			return false;
		}

		fTimeStamp = next->fTimeStamp;

		return true;
	}

private:
			Reference<Text>		fText;
			double				fWidth;
};

#endif // SET_TEXT_WIDTH_EDIT_H
