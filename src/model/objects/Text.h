/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef TEXT_H
#define TEXT_H

#include <GraphicsDefs.h>
#include <List.h>

#include "Styleable.h"
#include "TextLayout.h"

class Text : public Styleable {
public:
								Text(const rgb_color& color);
	virtual						~Text();

	// Object interface
	virtual	ObjectSnapshot*		Snapshot() const;

	virtual	const char*			DefaultName() const;

	virtual	bool				HitTest(const BPoint& canvasPoint);

	virtual	BRect				Bounds();

	// Text
			void				SetWidth(double width);
			double				Width();

			const TextLayout&	getTextLayout() const;

private:
			TextLayout			fTextLayout;
};

#endif // TEXT_H
