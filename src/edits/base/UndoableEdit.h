/*
 * Copyright 2006-2012 Stephan Aßmus <superstippi@gmx.de>
 * Distributed under the terms of the MIT License.
 */

#ifndef UNDOABLE_EDIT_H
#define UNDOABLE_EDIT_H

#include "Referenceable.h"

class BString;

class UndoableEdit : public Referenceable {
public:
								UndoableEdit();
	virtual						~UndoableEdit();

	virtual	status_t			InitCheck();

	virtual	status_t			Perform();
	virtual status_t			Undo();
	virtual status_t			Redo();

	virtual void				GetName(BString& name);

	virtual	bool				UndoesPrevious(const UndoableEdit* previous);
	virtual	bool				CombineWithNext(const UndoableEdit* next);
	virtual	bool				CombineWithPrevious(
									const UndoableEdit* previous);

protected:
			bigtime_t			fTimeStamp;
};

typedef Reference<UndoableEdit> UndoableEditRef;

#endif // UNDOABLE_EDIT_H
