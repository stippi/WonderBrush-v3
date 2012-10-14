/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "Font.h"

#include <stdio.h>


static const float kDefaultSize = 12.0;

// constructor
Font::Font()
	: fFamily("")
	, fStyle("")
	, fSize(kDefaultSize)
	, fRotation(0.0)
	, fShear(90.0)
	, fFalseBoldWidth(0.0)
	, fHinting(true)
	, fKerning(true)
	, fSpacing(B_BITMAP_SPACING)
	, fCachedFontHeightValid(false)
{
}

// constructor
Font::Font(const BFont& font)
	: fFamily("")
	, fStyle("")
	, fSize(font.Size())
	, fRotation(font.Rotation())
	, fShear(font.Shear())
	, fFalseBoldWidth(0.0)
	, fHinting(true)
	, fKerning(true)
	, fSpacing(font.Spacing())
	, fCachedFontHeightValid(true)
{
	font_family family;
	font_style style;
	font.GetFamilyAndStyle(&family, &style);
	fFamily = family;
	fStyle = style;
	font.GetHeight(&fCachedFontHeight);
}

// constructor
Font::Font(const Font& other)
	: fFamily(other.fFamily)
	, fStyle(other.fStyle)
	, fSize(other.fSize)
	, fRotation(other.fRotation)
	, fShear(other.fShear)
	, fFalseBoldWidth(other.fFalseBoldWidth)
	, fHinting(other.fHinting)
	, fKerning(other.fKerning)
	, fSpacing(other.fSpacing)
	, fCachedFontHeight(other.fCachedFontHeight)
	, fCachedFontHeightValid(other.fCachedFontHeightValid)
{
}

// constructor
Font::Font(const char* family, const char* style)
	: fFamily(family)
	, fStyle(style)
	, fSize(kDefaultSize)
	, fRotation(0.0)
	, fShear(90.0)
	, fFalseBoldWidth(0.0)
	, fHinting(true)
	, fKerning(true)
	, fSpacing(B_BITMAP_SPACING)
	, fCachedFontHeightValid(false)
{
}

// destructor
Font::~Font()
{
}

// operator=
Font&
Font::operator=(const Font& other)
{
	fFamily = other.fFamily;
	fStyle = other.fStyle;
	fSize = other.fSize;
	fRotation = other.fRotation;
	fShear = other.fShear;
	fFalseBoldWidth = other.fFalseBoldWidth;
	fHinting = other.fHinting;
	fKerning = other.fKerning;
	fSpacing = other.fSpacing;
	fCachedFontHeight = other.fCachedFontHeight;
	fCachedFontHeightValid = other.fCachedFontHeightValid;
	return *this;
}

// operator==
bool
Font::operator==(const Font& other) const
{
	return fSize == other.fSize
		&& fRotation == other.fRotation
		&& fShear == other.fShear
		&& fFalseBoldWidth == other.fFalseBoldWidth
		&& fHinting == other.fHinting
		&& fKerning == other.fKerning
		&& fSpacing == other.fSpacing
		&& fFamily == other.fFamily
		&& fStyle == other.fStyle;
}

// operator!=
bool
Font::operator!=(const Font& other) const
{
	return fSize != other.fSize
		|| fRotation != other.fRotation
		|| fShear != other.fShear
		|| fFalseBoldWidth != other.fFalseBoldWidth
		|| fHinting != other.fHinting
		|| fKerning != other.fKerning
		|| fSpacing != other.fSpacing
		|| fFamily != other.fFamily
		|| fStyle != other.fStyle;
}

// GetBFont
BFont
Font::GetBFont() const
{
	BFont font;
	font.SetFamilyAndStyle(fFamily.String(), fStyle.String());
	font.SetSize(fSize);
	font.SetRotation(fRotation);
	font.SetShear(fShear);
	font.SetSpacing(fSpacing);
	return font;
}

// SetFamilyAndStyle
status_t
Font::SetFamilyAndStyle(const font_family family, const font_style style)
{
	// TODO: ask FontManager if this font exists
	fFamily = family;
	fStyle = style;
	return B_OK;
}

// GetFamilyAndStyle
void
Font::GetFamilyAndStyle(font_family* family, font_style* style) const
{
	if (family)
		snprintf(*family, B_FONT_FAMILY_LENGTH, "%s", fFamily.String());
	if (style)
		snprintf(*style, B_FONT_STYLE_LENGTH, "%s", fStyle.String());
}

// SetSize
void
Font::SetSize(float size)
{
	if (fSize == size)
		return;
	fSize = size;
	fCachedFontHeightValid = false;
}

// SetRotation
void
Font::SetRotation(float rotation)
{
	fRotation = rotation;
}

// SetShear
void
Font::SetShear(float shear)
{
	fShear = shear;
}

// SetHinting
void
Font::SetHinting(bool hinting)
{
	fHinting = hinting;
}

// SetKerning
void
Font::SetKerning(bool kerning)
{
	fKerning = kerning;
}

// SetSpacing
void
Font::SetSpacing(uint8 spacing)
{
	fSpacing = spacing;
}

// GetHeight
void
Font::GetHeight(font_height* fh) const
{
	if (!fCachedFontHeightValid) {
		// TODO:
		BFont helper = GetBFont();
		helper.GetHeight(&fCachedFontHeight);
		fCachedFontHeightValid = true;
	}

	*fh = fCachedFontHeight;
}

// StringWidth
float
Font::StringWidth(const char *string) const
{
	// TODO:
	BFont helper = GetBFont();
	return helper.StringWidth(string);
}

