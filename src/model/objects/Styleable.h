/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>. All rights reserved.
 */
#ifndef STYLEABLE_H
#define STYLEABLE_H

#include <GraphicsDefs.h>

#include "Object.h"

class Style;

class Styleable : public Object {
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

	virtual	BRect				Area() const = 0;

private:
			Reference< ::Style>	fStyle;
};

#endif // STYLEABLE_H
