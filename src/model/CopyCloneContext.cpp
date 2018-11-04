/*
 * Copyright 2018, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "CopyCloneContext.h"

// constructor
CopyCloneContext::CopyCloneContext()
	: CloneContext()
{
}

// destructor
CopyCloneContext::~CopyCloneContext()
{
}

// ResolveResource
BaseObjectRef
CopyCloneContext::Clone(BaseObject* object)
{
	return BaseObjectRef(object->Clone(*this));
}
