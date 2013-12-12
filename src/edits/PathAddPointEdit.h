/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef PATH_ADD_POINT_EDIT_H
#define PATH_ADD_POINT_EDIT_H

#include "ShapeEdit.h"
#include "Path.h"
#include "PathMovePointEdit.h"

class PathAddPointEdit : public ShapeEdit {
public:
	PathAddPointEdit(Shape* shape, const PathRef& path, BPoint point,
			Selection* selection)
		: ShapeEdit(shape, selection)
		, fPath(path)
		, fIndex(-1)
		, fPoint(point)
		, fPointIn(point)
		, fPointOut(point)
		, fConnected(true)
	{
	}

	virtual ~PathAddPointEdit()
	{
	}

	virtual	status_t InitCheck()
	{
		return fPath.Get() != NULL ? B_OK : B_ERROR;
	}

	virtual	status_t Perform(EditContext& context)
	{
		SelectShape();
		fIndex = fPath->CountPoints();
		if (fPath->AddPoint(fPoint, fPointIn, fPointOut, fConnected))
			return B_OK;
		else
			return B_NO_MEMORY;
	}

	virtual status_t Undo(EditContext& context)
	{
		SelectShape();
		if (fPath->RemovePoint(fIndex))
			return B_OK;
		else
			return B_NO_MEMORY;
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

		// Store current path point state after the PathMovePointEdit
		// has performed
		fPath->GetPointsAt(fIndex, fPoint, fPointIn, fPointOut,
			&fConnected);

		fTimeStamp = next->TimeStamp();

		return true;
	}


	virtual void GetName(BString& name)
	{
		name << "Add control point";
	}

private:
			PathRef				fPath;
			int32				fIndex;
			BPoint				fPoint;
			BPoint				fPointIn;
			BPoint				fPointOut;
			bool				fConnected;
};

#endif // PATH_ADD_POINT_EDIT_H
