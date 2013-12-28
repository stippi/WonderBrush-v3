/*
 * Copyright 2009-2010, Stephan Aßmus <superstippi@gmx.de>. All rights reserved.
 */
#ifndef STYLEABLE_H
#define STYLEABLE_H

#include <GraphicsDefs.h>

#include "BoundedObject.h"
#include "Listener.h"

class Style;

class Styleable : public BoundedObject, public Listener {
public:
								Styleable();
								Styleable(const rgb_color& color);
								Styleable(const Styleable& other,
									CloneContext& context);
	virtual						~Styleable();

	// Object interface
	virtual	void				AddProperties(PropertyObject* object,
									uint32 flags = 0) const;
	virtual	bool				SetToPropertyObject(
									const PropertyObject* object,
									uint32 flags = 0);
	// Listener interface
	virtual	void				ObjectChanged(const Notifier* object);

	// Styleable
			void				SetStyle(::Style* style);
	inline	::Style*			Style() const
									{ return fStyle.Get(); }

private:
			Reference< ::Style>	fStyle;
};

#endif // STYLEABLE_H
