/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef STYLE_SET_FILL_PAINT_EDIT_H
#define STYLE_SET_FILL_PAINT_EDIT_H

#include <String.h>

#include "Paint.h"
#include "Style.h"
#include "UndoableEdit.h"

class StyleSetFillPaintEdit : public UndoableEdit {
public:
	StyleSetFillPaintEdit(Style* style, const Paint& newPaint)
		: UndoableEdit()
		, fStyle(style)
		, fFillPaint(newPaint)
	{
	}

	virtual ~StyleSetFillPaintEdit()
	{
	}

	virtual	status_t InitCheck()
	{
		return fStyle.Get() != NULL
			&& fStyle->FillPaint() != NULL
			&& *fStyle->FillPaint() != fFillPaint ? B_OK : B_ERROR;
	}

	virtual	status_t Perform(EditContext& context)
	{
		Paint oldPaint = *fStyle->FillPaint();
		fStyle->SetFillPaint(fFillPaint);
		fFillPaint = oldPaint;

		return B_OK;
	}

	virtual status_t Undo(EditContext& context)
	{
		return Perform(context);
	}

	virtual void GetName(BString& name)
	{
		name << "Change fill paint";
	}

	virtual	bool CombineWithNext(const UndoableEdit* _next)
	{
		const StyleSetFillPaintEdit* next
			= dynamic_cast<const StyleSetFillPaintEdit*>(_next);

		if (next == NULL || next->fStyle.Get() != fStyle.Get()
			|| next->fTimeStamp - fTimeStamp > 500000) {
			return false;
		}

		fTimeStamp = next->fTimeStamp;

		return true;
	}

private:
			StyleRef			fStyle;
			Paint				fFillPaint;
};

#endif // STYLE_SET_FILL_PAINT_EDIT_H
