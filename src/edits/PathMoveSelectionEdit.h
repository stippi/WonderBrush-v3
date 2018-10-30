/*
 * Copyright 2015, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef PATH_MOVE_SELECTION_EDIT_H
#define PATH_MOVE_SELECTION_EDIT_H

#include "ObjectEdit.h"
#include "Path.h"

class PathMoveSelectionEdit : public ObjectEdit<Shape> {
public:

	PathMoveSelectionEdit(Shape* shape, const PointSelection& points,
		BPoint offset, Selection* selection)
		: ObjectEdit(shape, selection)
		, fPointSelection(points)
		, fOffset(offset)
	{
	}

	virtual ~PathMoveSelectionEdit()
	{
	}

	virtual	status_t InitCheck()
	{
		return fPointSelection.Size() > 0 && fOffset != B_ORIGIN
			? B_OK : B_ERROR;
	}

	virtual	status_t Perform(EditContext& context)
	{
		SelectObject();

		// TODO: Stop Path notifications until all points are modified

		PointSelection::Iterator iterator = fPointSelection.GetIterator();
		while (iterator.HasNext()) {
			PathPoint pathPoint = iterator.Next();
			pathPoint.OffsetBy(fOffset);
		}

		fOffset = -fOffset;

		return B_OK;
	}

	virtual status_t Undo(EditContext& context)
	{
		return Perform(context);
	}

	virtual	bool CombineWithNext(const UndoableEdit* _next)
	{
		const PathMoveSelectionEdit* next
			= dynamic_cast<const PathMoveSelectionEdit*>(_next);

		if (next == NULL || next->fPointSelection != fPointSelection
			|| next->fTimeStamp - fTimeStamp > 250000) {
			return false;
		}

		fTimeStamp = next->fTimeStamp;
		fOffset += next->fOffset;

		return true;
	}

	virtual void GetName(BString& name)
	{
		name << "Move control points";
	}

private:
			PointSelection		fPointSelection;
			BPoint				fOffset;
};

#endif // PATH_MOVE_SELECTION_EDIT_H
