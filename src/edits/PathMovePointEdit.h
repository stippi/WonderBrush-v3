/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef PATH_MOVE_POINT_EDIT_H
#define PATH_MOVE_POINT_EDIT_H

#include "ObjectEdit.h"
#include "Path.h"

class PathMovePointEdit : public ObjectEdit<Shape> {
public:

	enum {
		MOVE_POINT					= 0,
		MOVE_POINT_IN				= 1,
		MOVE_POINT_OUT				= 2,
		MOVE_POINT_OUT_MIRROR_IN	= 3,
	};

	PathMovePointEdit(Shape* shape, const PathRef& path, int32 index,
		BPoint point, int32 mode, Selection* selection)
		: ObjectEdit(shape, selection)
		, fPath(path)
		, fIndex(index)
		, fPoint(point)
		, fMode(mode)
	{
	}

	virtual ~PathMovePointEdit()
	{
	}

	virtual	status_t InitCheck()
	{
		return fPath.Get() != NULL
			&& fIndex >= 0 && fIndex < fPath->CountPoints() ? B_OK : B_ERROR;
	}

	virtual	status_t Perform(EditContext& context)
	{
		SelectObject();

		BPoint point;
		switch (fMode) {
			case MOVE_POINT:
				fPath->GetPointAt(fIndex, point);
				fPath->SetPoint(fIndex, fPoint);
				break;
			case MOVE_POINT_IN:
				fPath->GetPointInAt(fIndex, point);
				fPath->SetPointIn(fIndex, fPoint);
				break;
			case MOVE_POINT_OUT:
				fPath->GetPointOutAt(fIndex, point);
				fPath->SetPointOut(fIndex, fPoint, false);
				break;
			case MOVE_POINT_OUT_MIRROR_IN:
				fPath->GetPointOutAt(fIndex, point);
				fPath->SetPointOut(fIndex, fPoint, true);
				break;
			default:
				return B_ERROR;
		}

		fPoint = point;

		return B_OK;
	}

	virtual status_t Undo(EditContext& context)
	{
		return Perform(context);
	}

	virtual	bool CombineWithNext(const UndoableEdit* _next)
	{
		const PathMovePointEdit* next
			= dynamic_cast<const PathMovePointEdit*>(_next);

		if (next == NULL || next->fPath != fPath
			|| next->fIndex != fIndex || next->fMode != fMode
			|| next->fTimeStamp - fTimeStamp > 500000) {
			return false;
		}

		fTimeStamp = next->fTimeStamp;

		return true;
	}

	virtual void GetName(BString& name)
	{
		name << "Move control point";
	}

	inline const PathRef& GetPath() const
	{
		return fPath;
	}

	inline int32 PointIndex() const
	{
		return fIndex;
	}

private:
			PathRef				fPath;
			int32				fIndex;
			BPoint				fPoint;
			int32				fMode;
};

#endif // PATH_MOVE_POINT_EDIT_H
