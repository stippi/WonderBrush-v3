/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef TEXT_H
#define TEXT_H

#include <GraphicsDefs.h>
#include <List.h>
#include <String.h>

#include "Font.h"
#include "Styleable.h"
#include "TextLayout.h"

class CharacterStyle;
class StyleRun;
class StyleRunList;

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

			void				SetAlignment(uint32 alignment);
			void				SetJustify(bool justify);

			void				SetText(const char* utf8String,
									const Font& font, rgb_color color);

			void				SetText(const char* utf8String,
									const Font& font, const StyleRef& style);

			const char*			GetText() const;

			int32				GetCharCount() const;

			void				Insert(int32 textOffset,
									const char* utf8String, const Font& font,
									const StyleRef& style);

			void				Remove(int32 textOffset, int32 length);

			void				SetStyle(int32 textOffset, int32 length,
									const Font& font, const StyleRef& style);

			void				SetFont(int32 textOffset, int32 length,
									const char* family, const char* style);

			void				SetSize(int32 textOffset, int32 length,
									double size);

			void				SetColor(int32 textOffset, int32 length,
									const rgb_color& color);

			const TextLayout&	getTextLayout() const;
			TextLayout&			getTextLayout();

			void				UpdateLayout();

private:
			BString				fText;
			int32				fCharCount;
			TextLayout			fTextLayout;
			StyleRunList*		fStyleRuns;
};

#endif // TEXT_H
