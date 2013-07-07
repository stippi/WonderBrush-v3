/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef SET_TEXT_ALIGNMENT_EDIT_H
#define SET_TEXT_ALIGNMENT_EDIT_H

#include <String.h>

#include "UndoableEdit.h"
#include "Text.h"

class SetTextAlignmentEdit : public UndoableEdit {
public:
	SetTextAlignmentEdit(Text* text, uint32 alignment)
		: UndoableEdit()
		, fText(text)
		, fAlignment(alignment)
	{
	}

	virtual ~SetTextAlignmentEdit()
	{
	}

	virtual	status_t InitCheck()
	{
		return fText.Get() != NULL && fText->Alignment() != fAlignment
			? B_OK : B_ERROR;
	}

	virtual	status_t Perform(EditContext& context)
	{
		uint32 previousAlignment = fText->Alignment();
		fText->SetAlignment(fAlignment);
		fAlignment = previousAlignment;

		return B_OK;
	}

	virtual status_t Undo(EditContext& context)
	{
		return Perform(context);
	}

	virtual void GetName(BString& name)
	{
		name << "Change text alignment";
	}

	virtual	bool CombineWithNext(const UndoableEdit* _next)
	{
		const SetTextAlignmentEdit* next
			= dynamic_cast<const SetTextAlignmentEdit*>(_next);

		if (next == NULL || next->fText != fText
			|| next->fTimeStamp - fTimeStamp > 500000) {
			return false;
		}

		fTimeStamp = next->fTimeStamp;

		return true;
	}

private:
			Reference<Text>		fText;
			uint32				fAlignment;
};

#endif // SET_TEXT_ALIGNMENT_EDIT_H
