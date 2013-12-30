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
	StyleSetFillPaintEdit(Style* style, const rgb_color& color)
		: UndoableEdit()
		, fStyle(style)
		, fColor(color)
		, fOldColor()
	{
	}

	virtual ~StyleSetFillPaintEdit()
	{
	}

	virtual	status_t InitCheck()
	{
		return fStyle.Get() != NULL
			&& fStyle->FillPaint() != NULL
			&& (fStyle->FillPaint()->Type() != Paint::COLOR
				|| fStyle->FillPaint()->Color() != fColor) ? B_OK : B_ERROR;
	}

	virtual	status_t Perform(EditContext& context)
	{
		fOldColor = fStyle->FillPaint()->Color();
		fStyle->FillPaint()->SetColor(fColor);

		return B_OK;
	}

	virtual status_t Undo(EditContext& context)
	{
		fStyle->FillPaint()->SetColor(fOldColor);

		return B_OK;
	}

	virtual status_t Redo(EditContext& context)
	{
		fStyle->FillPaint()->SetColor(fColor);

		return B_OK;
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
		fColor = next->fColor;

		return true;
	}

private:
			StyleRef			fStyle;
			rgb_color			fColor;
			rgb_color			fOldColor;
};

#endif // STYLE_SET_FILL_PAINT_EDIT_H
