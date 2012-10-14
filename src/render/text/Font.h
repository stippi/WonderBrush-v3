/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef FONT_H
#define FONT_H

#include <Font.h>
#include <String.h>

class Font {
public:
								Font();
								Font(const BFont& font);
								Font(const Font& other);
								Font(const char* family,
									const char* style);
	virtual						~Font();

			Font&				operator=(const Font& other);
			bool				operator==(const Font& other) const;
			bool				operator!=(const Font& other) const;

			BFont				GetBFont() const;

			const char*			Family() const
									{ return fFamily.String(); }
			const char*			Style() const
									{ return fStyle.String(); }

			status_t			SetFamilyAndStyle(const font_family family,
									const font_style style);
			void				GetFamilyAndStyle(font_family* family,
									font_style* style) const;

			void				SetSize(float size);
			float				Size() const
									{ return fSize; }

			void				SetRotation(float rotation);
			float				Rotation() const
									{ return fRotation; }

			void				SetShear(float shear);
			float				Shear() const
									{ return fShear; }

			void				SetFalseBoldWidth(float width);
			float				FalseBoldWidth() const
									{ return fFalseBoldWidth; }

			void				SetHinting(bool hinting);
			bool				Hinting() const
									{ return fHinting; }

			void				SetKerning(bool kerning);
			bool				Kerning() const
									{ return fKerning; }

			void				SetSpacing(uint8 spacing);
			uint8				Spacing() const
									{ return fSpacing; }

			void				GetHeight(font_height* fh) const;

			float				StringWidth(const char *string) const;

private:
			BString				fFamily;
			BString				fStyle;
			float				fSize;
			float				fRotation;
			float				fShear;
			float				fFalseBoldWidth;
			bool				fHinting;
			bool				fKerning;
			uint8				fSpacing;

	mutable	font_height			fCachedFontHeight;
	mutable	bool				fCachedFontHeightValid;
};

#endif // FONT_H
