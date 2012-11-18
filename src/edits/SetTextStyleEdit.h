/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef SET_TEXT_STYLE_EDIT_H
#define SET_TEXT_STYLE_EDIT_H

#include <String.h>

#include "UndoableEdit.h"
#include "StyleRun.h"
#include "StyleRunList.h"
#include "Text.h"

class SetTextStyleEdit : public UndoableEdit {
public:
	SetTextStyleEdit(Text* text, int32 textOffset, int32 length,
		const rgb_color& color)
		: UndoableEdit()
		, fCommandName("Change text color")
	{
		_Init(text, textOffset, length);
		if (fOldStyles == NULL || fNewStyles == NULL)
			return;

		::Style* style = new(std::nothrow) ::Style();
		if (style == NULL)
			return;

		style->SetFillPaint(Paint(color));

		StyleRef styleRef(style, true);

// TODO: Maybe make something like this possible:
//		for (in32 i = 0; i < fNewStyles->CountRuns(); i++) {
//			StyleRun* run = fNewStyles->StyleRunAt(i);
//			run->GetStyle()->SetStyle(style);
//		}

		textOffset = 0;
		while (length > 0) {
			// TODO: Make more efficient
			const StyleRun* run = fNewStyles->FindStyleRun(textOffset);

			CharacterStyle* characterStyle = new(std::nothrow) CharacterStyle(
				Font(run->GetStyle()->GetFont()), styleRef);

			if (characterStyle == NULL)
				return;

			CharacterStyleRef styleRef(characterStyle, true);

			StyleRun replaceRun(styleRef);
			replaceRun.SetLength(1);

			if (!fNewStyles->Insert(textOffset, replaceRun))
				return;

			fNewStyles->Remove(textOffset + 1, 1);
			styleRef.Detach();

			textOffset++;
			length--;
		}
	}

	SetTextStyleEdit(Text* text, int32 textOffset, int32 length,
		double fontSize)
		: UndoableEdit()
		, fCommandName("Change text size")
	{
		_Init(text, textOffset, length);
		if (fOldStyles == NULL || fNewStyles == NULL)
			return;

		textOffset = 0;
		while (length > 0) {
			// TODO: Make more efficient
			const StyleRun* run = fNewStyles->FindStyleRun(textOffset);

			CharacterStyle* characterStyle = new(std::nothrow) CharacterStyle(
				Font(run->GetStyle()->GetFont().getFamily(),
					run->GetStyle()->GetFont().getStyle(), fontSize,
					run->GetStyle()->GetFont().getScriptLevel()),
				run->GetStyle()->GetStyle());

			if (characterStyle == NULL)
				return;

			CharacterStyleRef styleRef(characterStyle, true);

			StyleRun replaceRun(styleRef);
			replaceRun.SetLength(1);

			if (!fNewStyles->Insert(textOffset, replaceRun))
				return;

			fNewStyles->Remove(textOffset + 1, 1);
			styleRef.Detach();

			textOffset++;
			length--;
		}
	}

	SetTextStyleEdit(Text* text, int32 textOffset, int32 length,
		const char* family, const char* style)
		: UndoableEdit()
		, fCommandName("Change text font")
	{
		_Init(text, textOffset, length);
		if (fOldStyles == NULL || fNewStyles == NULL)
			return;

		textOffset = 0;
		while (length > 0) {
			// TODO: Make more efficient
			const StyleRun* run = fNewStyles->FindStyleRun(textOffset);

			CharacterStyle* characterStyle = new(std::nothrow) CharacterStyle(
				Font(family, style, run->GetStyle()->GetFont().getSize(),
					run->GetStyle()->GetFont().getScriptLevel()),
				run->GetStyle()->GetStyle());

			if (characterStyle == NULL)
				return;

			CharacterStyleRef styleRef(characterStyle, true);

			StyleRun replaceRun(styleRef);
			replaceRun.SetLength(1);

			if (!fNewStyles->Insert(textOffset, replaceRun))
				return;

			fNewStyles->Remove(textOffset + 1, 1);
			styleRef.Detach();

			textOffset++;
			length--;
		}
	}

	virtual ~SetTextStyleEdit()
	{
		delete fOldStyles;
		delete fNewStyles;
		fText->RemoveReference();
	}

	virtual	status_t InitCheck()
	{
		return fText != NULL && fOldStyles != NULL && fNewStyles != NULL
			? B_OK : B_ERROR;
	}

	virtual	status_t Perform()
	{
		fText->ReplaceStyles(fOffset, fString.CountChars(), *fNewStyles);
		return B_OK;
	}

	virtual status_t Undo()
	{
		fText->ReplaceStyles(fOffset, fString.CountChars(), *fOldStyles);
		return B_OK;
	}

	virtual void GetName(BString& name)
	{
		name << fCommandName;
	}

	virtual	bool CombineWithNext(const UndoableEdit* _next)
	{
		const SetTextStyleEdit* next
			= dynamic_cast<const SetTextStyleEdit*>(_next);

		if (next == NULL || next->fText != fText
			|| next->fTimeStamp - fTimeStamp > 500000
			|| next->fOldStyles == NULL
			|| next->fOffset != fOffset
			|| next->fCommandName != fCommandName
			|| next->fString != fString) {
			return false;
		}

		*fNewStyles = *(next->fNewStyles);
		fTimeStamp = next->fTimeStamp;

		return true;
	}

private:
	void _Init(Text* text, int32 offset, int32 length)
	{
		fText = text;
		fText->AddReference();
		fOffset = offset;
		fString = fText->GetSubString(offset, length);
		fOldStyles = fText->GetStyleRuns(offset, length);
		if (fOldStyles != NULL)
			fNewStyles = new(std::nothrow) StyleRunList(*fOldStyles);
		else
			fNewStyles = NULL;
	}

private:
			Text*				fText;
			int32				fOffset;
			BString				fString;
			StyleRunList*		fOldStyles;
			StyleRunList*		fNewStyles;
			BString				fCommandName;
};

#endif // SET_TEXT_STYLE_EDIT_H
