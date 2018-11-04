/*
 * Copyright 2018, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef COPY_CLONE_CONTEXT_H
#define COPY_CLONE_CONTEXT_H

#include "CloneContext.h"

class CopyCloneContext : public CloneContext {
public:
								CopyCloneContext();
	virtual						~CopyCloneContext();

	virtual	BaseObjectRef		Clone(BaseObject* object);
};

#endif // COPY_CLONE_CONTEXT_H
