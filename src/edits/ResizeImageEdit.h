/*
 * Copyright 2015, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef RESIZE_IMAGE_EDIT_H
#define RESIZE_IMAGE_EDIT_H

#include <Rect.h>

#include "Document.h"
#include "UndoableEdit.h"

class ResizeImageEdit : public UndoableEdit {
 public:
	ResizeImageEdit(const Reference<Document> document, int32 newWidth,
			int32 newHeight)
		: UndoableEdit()
		, fDocument(document)
		, fWidth(newWidth)
		, fHeight(newHeight)
		, fTransformation(*fDocument->RootLayer())
	{
		BRect bounds = fDocument->Bounds();
		int32 width = bounds.IntegerWidth() + 1;
		int32 height = bounds.IntegerHeight() + 1;
		if (newWidth > 0 && newHeight > 0 && width > 0 && height > 0) {
			fTransformation.ScaleBy(B_ORIGIN,
				(double)newWidth / width, (double)newHeight / height);
		}
	}

	virtual ~ResizeImageEdit()
	{
	}

	virtual	status_t InitCheck()
	{
		BRect bounds = fDocument->Bounds();
		if ((bounds.IntegerWidth() + 1 == fWidth
				&& bounds.IntegerHeight() + 1 == fHeight)
			|| fWidth < 1 || fHeight < 1) {
			return B_ERROR;
		}
		return B_OK;
	}

	virtual	status_t Perform(EditContext& context)
	{
		BRect bounds = fDocument->Bounds();
		int32 previousWidth = bounds.IntegerWidth() + 1;
		int32 previousHeight = bounds.IntegerHeight() + 1;
		fDocument->SetBounds(BRect(0, 0, fWidth - 1, fHeight - 1));
		fWidth = previousWidth;
		fHeight = previousHeight;

		Transformable previousTransformation = *fDocument->RootLayer();
		fDocument->RootLayer()->SetTransformable(fTransformation);
		fTransformation = previousTransformation;

		return B_OK;
	}

	virtual status_t Undo(EditContext& context)
	{
		return Perform(context);
	}

	virtual void GetName(BString& name)
	{
		name << "Resize image";
	}

 private:
			Reference<Document>	fDocument;
			int32				fWidth;
			int32				fHeight;
			Transformable		fTransformation;
};

#endif // RESIZE_IMAGE_EDIT_H
