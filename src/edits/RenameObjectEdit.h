/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef RENAME_OBJECT_EDIT_H
#define RENAME_OBJECT_EDIT_H

#include "BaseObject.h"
#include "UndoableEdit.h"

class RenameObjectEdit : public UndoableEdit {
public:
	RenameObjectEdit(BaseObject* object, const char* newName)
		: UndoableEdit()
		, fObject(object)
		, fNextName(newName)
	{
	}

	virtual ~RenameObjectEdit()
	{
	}

	virtual	status_t InitCheck()
	{
		return fObject && fNextName != fObject->Name() ? B_OK : B_ERROR;
	}

	virtual	status_t Perform(EditContext& context)
	{
		BString oldName = fObject->Name();
		fObject->SetName(fNextName.String());
		fNextName = oldName;

		return B_OK;
	}

	virtual status_t Undo(EditContext& context)
	{
		return Perform(context);
	}

	virtual void GetName(BString& name)
	{
		name << "Rename Object";
	}

private:
			BaseObject*			fObject;
			BString				fNextName;
};

#endif // RENAME_OBJECT_EDIT_H
