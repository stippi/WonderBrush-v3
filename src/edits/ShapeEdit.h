/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef SHAPE_EDIT_H
#define SHAPE_EDIT_H

#include <String.h>

#include "Selection.h"
#include "Shape.h"
#include "UndoableEdit.h"

class ShapeEdit : public UndoableEdit, public Selection::Controller {
public:
	ShapeEdit(Shape* shape, Selection* selection)
		: UndoableEdit()
		, fShape(shape)
		, fSelection(selection)
	{
	}

	virtual ~ShapeEdit()
	{
	}

protected:
	void SelectShape()
	{
		if (fSelection == NULL || fShape.Get() == NULL)
			return;

		fSelection->Select(Selectable(fShape.Get()), this, false);
	}

private:
			Reference<Shape>	fShape;
			Selection*			fSelection;
};

#endif // SHAPE_EDIT_H
