/*
 * Copyright 2009-2010, Stephan Aßmus <superstippi@gmx.de>. All rights reserved.
 */
#ifndef STYLEABLE_H
#define STYLEABLE_H

#include <GraphicsDefs.h>

#include "BoundedObject.h"

class Style;

class Styleable : public BoundedObject {
public:
								Styleable();
								Styleable(const rgb_color& color);
	virtual						~Styleable();

	// Object interface
	virtual	void				AddProperties(PropertyObject* object) const;
	virtual	bool				SetToPropertyObject(
									const PropertyObject* object);
	// Styleable
			void				SetStyle(::Style* style);
	inline	::Style*			Style() const
									{ return fStyle.Get(); }

private:
			Reference< ::Style>	fStyle;
};

#endif // STYLEABLE_H
