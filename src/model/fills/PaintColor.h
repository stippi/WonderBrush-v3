/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef PAINT_COLOR_H
#define PAINT_COLOR_H


#include "Paint.h"

#include <GraphicsDefs.h>


class PaintColor : public Paint {
public:
								PaintColor();
								PaintColor(const PaintColor& other);
								PaintColor(const rgb_color& color);
								PaintColor(BMessage* archive);

	virtual						~PaintColor();

	// BaseObject interface
	virtual	const char*			DefaultName() const;

	// Paint interface
	virtual	status_t			Archive(BMessage* into,
									bool deep = true) const;

	virtual	Paint*				Clone() const;
	virtual	bool				HasTransparency() const;
	virtual	uint32				Type() const
									{ return COLOR; }
	virtual	size_t				HashKey() const;

	// PaintColor
			bool				operator==(const PaintColor& other) const;
			void				SetColor(const rgb_color& color);
	inline	rgb_color			Color() const
									{ return fColor; }

private:
			rgb_color			fColor;
};

#endif	// PAINT_COLOR_H
