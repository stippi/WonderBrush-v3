/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef PATH_TOGGLE_SHARP_EDIT_H
#define PATH_TOGGLE_SHARP_EDIT_H

#include "ShapeEdit.h"
#include "Path.h"
#include "PathMovePointEdit.h"

class PathToggleSharpEdit : public ShapeEdit {
public:
	PathToggleSharpEdit(Shape* shape, const PathRef& path, int32 index,
			bool resetPoint, Selection* selection)
		: ShapeEdit(shape, selection)
		, fPath(path)
		, fIndex(-1)
		, fResetPoint(resetPoint)
	{
		if (path.Get() != NULL && index >= 0 && index < path->CountPoints()) {
			fIndex = index;
			path->GetPointsAt(fIndex, fPoint, fPointIn, fPointOut, &fConnected);
		}
	}

	virtual ~PathToggleSharpEdit()
	{
	}

	virtual	status_t InitCheck()
	{
		return fIndex >= 0/* && (fPoint != fPointIn || fPoint != fPointOut)*/
			? B_OK : B_ERROR;
	}

	virtual	status_t Perform(EditContext& context)
	{
		SelectShape();
		
		if (fResetPoint)
			fPath->SetPoint(fIndex, fPoint, fPoint, fPoint, true);
		else
			fPath->SetPoint(fIndex, fPoint, fPointIn, fPointOut, false);
		
		return B_OK;
	}

	virtual status_t Undo(EditContext& context)
	{
		SelectShape();

		BPoint point(fPoint);
		BPoint pointIn(fPointIn);
		BPoint pointOut(fPointOut);
		bool connected(fConnected);
		
		fPath->GetPointsAt(fIndex, fPoint, fPointIn, fPointOut, &fConnected);
		fPath->SetPoint(fIndex, point, pointIn, pointOut, connected);
		
		return B_OK;
	}

	virtual status_t Redo(EditContext& context)
	{
		return Undo(context);
	}

	virtual	bool CombineWithNext(const UndoableEdit* _next)
	{
		const PathMovePointEdit* next
			= dynamic_cast<const PathMovePointEdit*>(_next);

		if (next == NULL || next->GetPath() != fPath
			|| next->PointIndex() != fIndex
			|| next->TimeStamp() - fTimeStamp > 500000) {
			return false;
		}

		fTimeStamp = next->TimeStamp();

		return true;
	}


	virtual void GetName(BString& name)
	{
		name << "Modify control point";
	}

private:
			PathRef				fPath;

			int32				fIndex;
			bool				fResetPoint;

			BPoint				fPoint;
			BPoint				fPointIn;
			BPoint				fPointOut;
			bool				fConnected;
};

#endif // PATH_TOGGLE_SHARP_EDIT_H
