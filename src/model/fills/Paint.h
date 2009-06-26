/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef PAINT_H
#define PAINT_H


#include "BaseObject.h"

class Paint : public BaseObject {
public:
	enum {
		NONE		= 0,
		COLOR		= 1,
		GRADIENT	= 2,
		PATTERN		= 3
	};

								Paint();
								Paint(const Paint& other);
								Paint(BMessage* archive);

	virtual						~Paint();

	virtual	status_t			Archive(BMessage* into,
									bool deep = true) const;

	virtual	Paint*				Clone() const = 0;
	virtual	bool				HasTransparency() const = 0;
	virtual	uint32				Type() const = 0;
};

#endif	// PAINT_H
