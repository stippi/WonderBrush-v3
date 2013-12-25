/*
 * Copyright 2012-2013, Stephan Aßmus <superstippi@gmx.de>.
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

enum {
	TEXT_ALIGNMENT_LEFT		= 0,
	TEXT_ALIGNMENT_CENTER	= 1,
	TEXT_ALIGNMENT_RIGHT	= 2,
	TEXT_ALIGNMENT_JUSTIFY	= 3,
};

class Text : public Styleable {
public:
								Text(const rgb_color& color);
								Text(const Text& other,
									ResourceResolver& resolver);
	virtual						~Text();

	// BaseObject interface
	virtual	BaseObject*			Clone(ResourceResolver& resolver) const;
	virtual	const char*			DefaultName() const;

	// Object interface
	virtual	ObjectSnapshot*		Snapshot() const;

	virtual	bool				HitTest(const BPoint& canvasPoint);

	virtual	BRect				Bounds();

	// Text
			void				SetWidth(double width);
			double				Width();

			void				SetAlignment(uint32 alignment);
			uint32				Alignment() const;

			void				SetGlyphSpacing(double spacing);
			double				GlyphSpacing() const;

			void				SetText(const char* utf8String,
									const Font& font, rgb_color color);

			void				SetText(const char* utf8String,
									const Font& font, const StyleRef& style);

			const BString&		GetText() const;

			int32				GetCharCount() const;

			void				Insert(int32 textOffset,
									const char* utf8String, const Font& font,
									rgb_color color);

			void				Insert(int32 textOffset,
									const char* utf8String, const Font& font,
									const StyleRef& style);

			void				Insert(int32 textOffset,
									const char* utf8String, const Font& font,
									double glyphSpacing, double fauxWeight,
									double fauxItalic, const StyleRef& style);

			void				Insert(int32 textOffset,
									const BString& utf8String,
									const StyleRunList& styleRuns);

			void				Append(const char* utf8String, const Font& font,
									rgb_color color);

			void				Append(const char* utf8String, const Font& font,
									const StyleRef& style);

			void				Append(const char* utf8String, const Font& font,
									double glyphSpacing, double fauxWeight,
									double fauxItalic, const StyleRef& style);

			void				Append(const BString& utf8String,
									const StyleRunList& styleRuns);

			void				ReplaceStyles(int32 textOffset, int32 length,
									const StyleRunList& styleRuns);

			void				Remove(int32 textOffset, int32 length);

			BString				GetSubString(int32 textOffset,
									int32 length) const;
			StyleRunList*		GetStyleRuns(int32 textOffset,
									int32 length) const;

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
