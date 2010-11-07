/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#ifndef OBJECT_ADDED_COMMAND_H
#define OBJECT_ADDED_COMMAND_H

#include "Command.h"
#include "Selection.h"

class Layer;
class Object;

class ObjectAddedCommand : public Command, public Selection::Controller {
public:
								ObjectAddedCommand(Object* object,
									Selection* selection);
	virtual						~ObjectAddedCommand();

	virtual	status_t			InitCheck();

	virtual	status_t			Perform();
	virtual status_t			Undo();
	virtual	status_t			Redo();

	virtual void				GetName(BString& name);

private:
			Object*				fObject;

			Layer*				fInsertionLayer;
			int32				fInsertionIndex;

			Selection*			fSelection;
};

#endif // OBJECT_ADDED_COMMAND_H
