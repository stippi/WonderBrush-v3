/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef REMOVE_TEXT_COMMAND_H
#define REMOVE_TEXT_COMMAND_H

#include <String.h>

#include "Command.h"
#include "StyleRunList.h"
#include "Text.h"

class RemoveTextCommand : public Command {
public:
	RemoveTextCommand(Text* text, int32 textOffset, int32 length)
		: Command()
		, fText(text)
		, fOffset(textOffset)
		, fLength(length)
		, fRemovedText()
		, fRemovedStyles(NULL)
	{
	}

	virtual ~RemoveTextCommand()
	{
		delete fRemovedStyles;
	}

	virtual	status_t InitCheck()
	{
		return fText != NULL && fLength > 0 ? B_OK : B_ERROR;
	}

	virtual	status_t Perform()
	{
		fRemovedText = fText->GetSubString(fOffset, fLength);

		delete fRemovedStyles;
		fRemovedStyles = fText->GetStyleRuns(fOffset, fLength);

		fText->Remove(fOffset, fLength);

		return B_OK;
	}

	virtual status_t Undo()
	{
		if (fRemovedStyles == NULL)
			return B_ERROR;

		fText->Insert(fOffset, fRemovedText, *fRemovedStyles);

		return B_OK;
	}

	virtual void GetName(BString& name)
	{
		name << "Remove text";
	}

	virtual	bool CombineWithNext(const Command* _next)
	{
		const RemoveTextCommand* next
			= dynamic_cast<const RemoveTextCommand*>(_next);

		if (next == NULL || next->fText != fText
			|| next->fTimeStamp - fTimeStamp > 500000
			|| next->fRemovedStyles == NULL
			|| (next->fOffset != fOffset
				&& next->fOffset + next->fLength != fOffset)) {
			return false;
		}

		if (next->fOffset == fOffset) {
			fLength += next->fLength;
			fRemovedStyles->Insert(fLength, *next->fRemovedStyles);
			fRemovedText.Append(next->fRemovedText);
		} else {
			fOffset -= next->fLength;
			fRemovedStyles->Insert(0, *next->fRemovedStyles);
			fRemovedText.Prepend(next->fRemovedText);
		}
		fTimeStamp = next->fTimeStamp;

		return true;
	}

private:
			Text*				fText;
			int32				fOffset;
			int32				fLength;
			BString				fRemovedText;
			StyleRunList*		fRemovedStyles;
};

#endif // REMOVE_TEXT_COMMAND_H
