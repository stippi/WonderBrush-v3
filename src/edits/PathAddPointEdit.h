/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef PATH_ADD_POINT_EDIT_H
#define PATH_ADD_POINT_EDIT_H

#include "ShapeEdit.h"
#include "Path.h"

class PathAddPointEdit : public ShapeEdit {
public:
	PathAddPointEdit(Shape* shape, const PathRef& path, BPoint point,
			Selection* selection)
		: ShapeEdit(shape, selection)
		, fPath(path)
		, fIndex(-1)
		, fPoint(point)
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
		if (fPath->AddPoint(fPoint))
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

	virtual void GetName(BString& name)
	{
		name << "Add control point";
	}

private:
			PathRef				fPath;
			int32				fIndex;
			BPoint				fPoint;
};

#endif // PATH_ADD_POINT_EDIT_H
