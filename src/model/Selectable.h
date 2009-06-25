/*
 * Copyright 2007-2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef SELECTABLE_H
#define SELECTABLE_H

#include "BaseObject.h"
#include "Referenceable.h"

class Selectable : public Reference<BaseObject> {
public:
								Selectable();
								Selectable(BaseObject* object);
								Selectable(const Selectable& other);
	virtual						~Selectable();
};

#endif // SELECTABLE_H
