/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#ifndef ADD_OBJECTS_COMMAND_H
#define ADD_OBJECTS_COMMAND_H

#include "Command.h"
#include "Selection.h"

class Layer;
class Object;

class AddObjectsCommand : public Command, public Selection::Controller {
public:
								AddObjectsCommand(Object** objects,
									int32 objectCount, Layer* insertionLayer,
									int32 insertionIndex,
									Selection* selection);
	virtual						~AddObjectsCommand();

	virtual	status_t			InitCheck();

	virtual	status_t			Perform();
	virtual status_t			Undo();

	virtual void				GetName(BString& name);

private:
			Object**			fObjects;
			int32				fObjectCount;
		
			Layer*				fInsertionLayer;
			int32				fInsertionIndex;
		
			Selection*			fSelection;
};

#endif // ADD_OBJECTS_COMMAND_H
