/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef RENAME_OBJECT_ACTION_H
#define RENAME_OBJECT_ACTION_H

#include "BaseObject.h"
#include "Command.h"

class RenameObjectCommand : public Command {
public:
	RenameObjectCommand(BaseObject* object, const char* newName)
		: Command()
		, fObject(object)
		, fNextName(newName)
	{
	}

	virtual ~RenameObjectCommand()
	{
	}

	virtual	status_t InitCheck()
	{
		return fObject && fNextName != fObject->Name() ? B_OK : B_ERROR;
	}

	virtual	status_t Perform()
	{
		BString oldName = fObject->Name();
		fObject->SetName(fNextName.String());
		fNextName = oldName;
	
		return B_OK;
	}

	virtual status_t Undo()
	{
		return Perform();
	}

	virtual void GetName(BString& name)
	{
		name << "Rename Object";
	}

private:
			BaseObject*			fObject;
			BString				fNextName;
};

#endif // RENAME_OBJECT_ACTION_H
