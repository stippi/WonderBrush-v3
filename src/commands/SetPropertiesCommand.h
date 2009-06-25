/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef SET_PROPERTIES_COMMAND_H
#define SET_PROPERTIES_COMMAND_H

#include "Command.h"

class BaseObject;
class PropertyObject;

class SetPropertiesCommand : public Command {
public:
								SetPropertiesCommand(BaseObject** objects,
									int32 objectCount,
									PropertyObject* previous,
									PropertyObject* current);
	virtual						~SetPropertiesCommand();

	virtual	status_t			InitCheck();

	virtual	status_t			Perform();
	virtual status_t			Undo();

	virtual void				GetName(BString& name);

private:
	BaseObject**				fObjects;
	int32						fObjectCount;

	PropertyObject*				fOldProperties;
	PropertyObject*				fNewProperties;
};

#endif // SET_PROPERTIES_COMMAND_H
