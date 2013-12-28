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
								Rect(const Rect& other,
									CloneContext& context);
	virtual						~Rect();

	// BaseObject interface
	virtual	BaseObject*			Clone(CloneContext& context) const;
	virtual	const char*			DefaultName() const;

	// Object interface
	virtual	ObjectSnapshot*		Snapshot() const;

	virtual	bool				HitTest(const BPoint& canvasPoint);

	// Styleable interface
	virtual	BRect				Bounds();

	// Rect
			void				SetArea(const BRect& area);
			BRect				Area() const;

			void				SetRoundCornerRadius(double radius);
			double				RoundCornerRadius() const;

private:
			BRect				fArea;
			double				fRoundCornerRadius;
};

#endif // RECT_H
