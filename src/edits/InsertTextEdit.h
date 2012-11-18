/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef INSERT_TEXT_EDIT_H
#define INSERT_TEXT_EDIT_H

#include <String.h>

#include "UndoableEdit.h"
#include "Font.h"
#include "Style.h"
#include "Text.h"

class InsertTextEdit : public UndoableEdit {
public:
	InsertTextEdit(Text* text, int32 textOffset, const char* utf8String,
		const Font& font, const StyleRef& style)
		: UndoableEdit()
		, fText(text)
		, fOffset(textOffset)
		, fString(utf8String)
		, fFont(font)
		, fStyleRef(style)
	{
	}

	virtual ~InsertTextEdit()
	{
	}

	virtual	status_t InitCheck()
	{
		return fText != NULL && fString.Length() > 0 ? B_OK : B_ERROR;
	}

	virtual	status_t Perform()
	{
		fText->Insert(fOffset, fString.String(), fFont, fStyleRef);

		return B_OK;
	}

	virtual status_t Undo()
	{
		fText->Remove(fOffset, fString.CountChars());

		return B_OK;
	}

	virtual void GetName(BString& name)
	{
		name << "Insert text";
	}

	virtual	bool CombineWithNext(const UndoableEdit* _next)
	{
		const InsertTextEdit* next
			= dynamic_cast<const InsertTextEdit*>(_next);

		if (next == NULL || next->fText != fText
			|| next->fString == " " || next->fString == "\n"
			|| next->fTimeStamp - fTimeStamp > 500000
			|| next->fOffset != fOffset + fString.CountChars()
			|| next->fFont != fFont
			|| *next->fStyleRef.Get() != *fStyleRef.Get()) {
			return false;
		}

		fString.Append(next->fString);
		fTimeStamp = next->fTimeStamp;

		return true;
	}

private:
			Text*				fText;
			int32				fOffset;
			BString				fString;
			Font				fFont;
			StyleRef			fStyleRef;
};

#endif // INSERT_TEXT_EDIT_H
