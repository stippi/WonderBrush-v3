/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef RESOURCE_RESOLVER_H
#define RESOURCE_RESOLVER_H

#include "BaseObject.h"

class CloneContext {
public:
								CloneContext();
	virtual						~CloneContext();

	virtual	BaseObjectRef		Clone(BaseObject* object);

	// Convenience method for cloning derived BaseObjects
	template<class BaseObjectType>
	void Clone(BaseObject* object, Reference<BaseObjectType>& reference)
	{
		BaseObjectRef ref = Clone(object);
		BaseObjectType* typedObject = dynamic_cast<BaseObjectType*>(ref.Get());
		reference.SetTo(typedObject);
	}
};

#endif // RESOURCE_RESOLVER_H
