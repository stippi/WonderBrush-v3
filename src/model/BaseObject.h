/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>.
 * Distributed under the terms of the MIT License.
 */

#ifndef BASE_OBJECT_H
#define BASE_OBJECT_H

#include <String.h>

#include "CommonPropertyIDs.h"
#include "Notifier.h"
#include "Property.h"
#include "PropertyObject.h"
#include "Referenceable.h"

class BMessage;

class BaseObject : public Notifier, public Referenceable {
public:
								BaseObject();
								BaseObject(const BaseObject& other);
								BaseObject(BMessage* archive);
	virtual						~BaseObject();

	// BaseObject
	virtual	status_t			Unarchive(const BMessage* archive);
	virtual status_t			Archive(BMessage* into,
										bool deep = true) const;

			PropertyObject*		MakePropertyObject() const;
	virtual	void				AddProperties(PropertyObject* object) const;
	virtual	bool				SetToPropertyObject(
									const PropertyObject* object);

	virtual	const char*			DefaultName() const = 0;
			void				SetName(const char* name);
			const char*			Name() const;

private:
			BString				fName;
};

#endif // BASE_OBJECT_H
