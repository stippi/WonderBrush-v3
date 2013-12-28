/*
 * Copyright 2012-2013 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "CharacterStyle.h"

#include "CloneContext.h"

// constructor
CharacterStyle::CharacterStyle(const Font& font, double glyphSpacing,
		double fauxWeight, double fauxItalic, const StyleRef& style)
	: fFont(font)
	, fGlyphSpacing(glyphSpacing)
	, fFauxWeight(fauxWeight)
	, fFauxItalic(fauxItalic)
	, fStyle(style)
{
}

// constructor
CharacterStyle::CharacterStyle(const CharacterStyle& other)
	: fFont(other.fFont)
	, fGlyphSpacing(other.fGlyphSpacing)
	, fFauxWeight(other.fFauxWeight)
	, fFauxItalic(other.fFauxItalic)
	, fStyle(other.fStyle.Get())
{
}

// constructor
CharacterStyle::CharacterStyle(const CharacterStyle& other,
		CloneContext& context)
	: fFont(other.fFont)
	, fGlyphSpacing(other.fGlyphSpacing)
	, fFauxWeight(other.fFauxWeight)
	, fFauxItalic(other.fFauxItalic)
	, fStyle()
{
	context.Clone(other.fStyle.Get(), fStyle);
}

// ==
bool
CharacterStyle::operator==(const CharacterStyle& other) const
{
	if (this == &other)
		return true;
	return fFont == other.fFont
		&& fGlyphSpacing == other.fGlyphSpacing
		&& fFauxWeight == other.fFauxWeight
		&& fFauxItalic == other.fFauxItalic
		&& *(fStyle.Get()) == *(other.fStyle.Get());
}

// !=
bool
CharacterStyle::operator!=(const CharacterStyle& other) const
{
	return *this != other;
}

// SetFont
CharacterStyle
CharacterStyle::SetFont(const Font& font) const
{
	CharacterStyle style(*this);
	style.fFont = font;
	return style;
}

// SetGlyphSpacing
CharacterStyle
CharacterStyle::SetGlyphSpacing(double spacing) const
{
	CharacterStyle style(*this);
	style.fGlyphSpacing = spacing;
	return style;
}

// SetFauxWeight
CharacterStyle
CharacterStyle::SetFauxWeight(double fauxWeight) const
{
	CharacterStyle style(*this);
	style.fFauxWeight = fauxWeight;
	return style;
}

// SetFauxItalic
CharacterStyle
CharacterStyle::SetFauxItalic(double fauxItalic) const
{
	CharacterStyle style(*this);
	style.fFauxItalic = fauxItalic;
	return style;
}

// SetStyle
CharacterStyle
CharacterStyle::SetStyle(const StyleRef& style) const
{
	CharacterStyle derived(*this);
	derived.fStyle = style;
	return derived;
}

