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
class CloneContext;

class BaseObject : public Notifier, public Referenceable {
public:
	enum {
		DONT_ADD_NAME = 1 << 2,
	};

public:
								BaseObject();
								BaseObject(const BaseObject& other);
								BaseObject(BMessage* archive);
	virtual						~BaseObject();

	// BaseObject
			BaseObject*			Clone() const;
	virtual BaseObject*			Clone(CloneContext& context) const = 0;

	virtual	status_t			Unarchive(const BMessage* archive);
	virtual status_t			Archive(BMessage* into,
										bool deep = true) const;

			PropertyObject*		MakePropertyObject() const;
	virtual	void				AddProperties(PropertyObject* object,
									uint32 flags = 0) const;
	virtual	bool				SetToPropertyObject(
									const PropertyObject* object,
									uint32 flags = 0);

	virtual	const char*			DefaultName() const = 0;
			void				SetName(const char* name);
			const char*			Name() const;
			const BString&		GivenName() const;

private:
			BString				fName;
};

typedef Reference<BaseObject> BaseObjectRef;

#endif // BASE_OBJECT_H
