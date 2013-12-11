/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef PATH_TOGGLE_CLOSED_EDIT_H
#define PATH_TOGGLE_CLOSED_EDIT_H

#include "ShapeEdit.h"
#include "Path.h"

class PathToggleClosedEdit : public ShapeEdit {
public:
	PathToggleClosedEdit(Shape* shape, const PathRef& path,
			Selection* selection)
		: ShapeEdit(shape, selection)
		, fPath(path)
	{
	}

	virtual ~PathToggleClosedEdit()
	{
	}

	virtual	status_t InitCheck()
	{
		return fPath.Get() != NULL ? B_OK : B_ERROR;
	}

	virtual	status_t Perform(EditContext& context)
	{
		SelectShape();
		fPath->SetClosed(!fPath->IsClosed());
		return B_OK;
	}

	virtual status_t Undo(EditContext& context)
	{
		return Perform(context);
	}

	virtual void GetName(BString& name)
	{
		name << "Toggle path closed";
	}

private:
			PathRef				fPath;
};

#endif // PATH_TOGGLE_CLOSED_EDIT_H
