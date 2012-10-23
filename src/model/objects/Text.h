/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef TEXT_H
#define TEXT_H

#include <GraphicsDefs.h>
#include <List.h>
#include <String.h>

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
			void				SetFont(const char* fontFilePath, double size);
	
			void				SetWidth(double width);
			double				Width();

			void				SetAlignment(uint32 alignment);
			void				SetJustify(bool justify);

			void				SetText(const char* utf8String);
			const char*			GetText() const;

			const TextLayout&	getTextLayout() const;

private:
			BString				fText;
			TextLayout			fTextLayout;
};

#endif // TEXT_H
