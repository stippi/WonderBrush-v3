/*
 * Copyright 2007-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef RECT_H
#define RECT_H

#include <GraphicsDefs.h>
#include <List.h>
#include <Rect.h>

#include "Styleable.h"

class Rect : public Styleable {
public:
								Rect();
								Rect(const BRect& area,
									const rgb_color& color);
	virtual						~Rect();

	// Object interface
	virtual	ObjectSnapshot*		Snapshot() const;

	virtual	const char*			DefaultName() const;

	virtual	bool				HitTest(const BPoint& canvasPoint);

	// Rect
			void				SetArea(const BRect& area);
			BRect				Area() const;
	virtual	BRect				Bounds();

private:
			BRect				fArea;
};

#endif // RECT_H
