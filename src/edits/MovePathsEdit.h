/*
 * Copyright 2010-2018, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#ifndef MOVE_PATHS_EDIT_H
#define MOVE_PATHS_EDIT_H

#include "UndoableEdit.h"
#include "Selection.h"

class PathInstance;
class Shape;

class MovePathsEdit : public UndoableEdit, public Selection::Controller {
public:
								MovePathsEdit(PathInstance** paths,
									int32 objectCount, Shape* insertionShape,
									int32 insertionIndex,
									Selection* selection);
	virtual						~MovePathsEdit();

	virtual	status_t			InitCheck();

	virtual	status_t			Perform(EditContext& context);
	virtual status_t			Undo(EditContext& context);

	virtual void				GetName(BString& name);

private:
			PathInstance**		fPaths;
			int32				fPathCount;

			struct PositionInfo {
				Shape*			parent;
				int32			index;
			};
			PositionInfo*		fOldPositions;

			Shape*				fInsertionShape;
			int32				fInsertionIndex;

			Selection*			fSelection;
};

#endif // MOVE_PATHS_EDIT_H
