/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "CloneContext.h"

// constructor
CloneContext::CloneContext()
{
}

// destructor
CloneContext::~CloneContext()
{
}

// ResolveResource
BaseObjectRef
CloneContext::Clone(BaseObject* object)
{
	return BaseObjectRef(object);
}
