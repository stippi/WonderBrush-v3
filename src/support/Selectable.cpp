/*
 * Copyright 2007-2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "Selectable.h"

// constructor
Selectable::Selectable()
	:
	Reference<BaseObject>()
{
}

// constructor
Selectable::Selectable(BaseObject* object)
	:
	Reference<BaseObject>(object)
{
}

// constructor
Selectable::Selectable(const Selectable& other)
	:
	Reference<BaseObject>(other)
{
}

// destructor
Selectable::~Selectable()
{
}


