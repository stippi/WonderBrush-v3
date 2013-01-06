/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef SET_GLYPH_SPACING_EDIT_H
#define SET_GLYPH_SPACING_EDIT_H

#include <String.h>

#include "UndoableEdit.h"
#include "Text.h"

class SetGlyphSpacingEdit : public UndoableEdit {
public:
	SetGlyphSpacingEdit(Text* text, double spacing)
		: UndoableEdit()
		, fText(text)
		, fSpacing(spacing)
	{
		fText->AddReference();
	}

	virtual ~SetGlyphSpacingEdit()
	{
		fText->RemoveReference();
	}

	virtual	status_t InitCheck()
	{
		return fText->GlyphSpacing() != fSpacing ? B_OK : B_ERROR;
	}

	virtual	status_t Perform()
	{
		double previousSpacing = fText->GlyphSpacing();
		fText->SetGlyphSpacing(fSpacing);
		fSpacing = previousSpacing;

		return B_OK;
	}

	virtual status_t Undo()
	{
		return Perform();
	}

	virtual void GetName(BString& name)
	{
		name << "Change glyph spacing";
	}

	virtual	bool CombineWithNext(const UndoableEdit* _next)
	{
		const SetGlyphSpacingEdit* next
			= dynamic_cast<const SetGlyphSpacingEdit*>(_next);

		if (next == NULL || next->fText != fText
			|| next->fTimeStamp - fTimeStamp > 500000) {
			return false;
		}

		fTimeStamp = next->fTimeStamp;

		return true;
	}

private:
			Text*				fText;
			double				fSpacing;
};

#endif // SET_GLYPH_SPACING_EDIT_H
