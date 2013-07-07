/*
 * Copyright 2010-2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#ifndef REMOVE_OBJECTS_EDIT_H
#define REMOVE_OBJECTS_EDIT_H

#include "List.h"
#include "UndoableEdit.h"
#include "Selection.h"

class Layer;
class Object;

class RemoveObjectsEdit : public UndoableEdit, public Selection::Controller {
public:
	typedef List<Reference<Object>, false, 1> ObjectList;

public:
								RemoveObjectsEdit(const ObjectList& objects,
									Selection* selection);
	virtual						~RemoveObjectsEdit();

	virtual	status_t			InitCheck();

	virtual	status_t			Perform(EditContext& context);
	virtual status_t			Undo(EditContext& context);

	virtual void				GetName(BString& name);

private:
			bool				_ObjectIsDistantChildOf(const Object* object,
									const Layer* layer) const;

private:
			ObjectList			fObjects;

			struct PositionInfo {
				Layer*			parent;
				int32			index;
			};
			PositionInfo*		fOldPositions;

			Selection*			fSelection;
};

#endif // REMOVE_OBJECTS_EDIT_H
