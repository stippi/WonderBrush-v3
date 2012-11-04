/*
 * Copyright 2012 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "CharacterStyle.h"

// constructor
CharacterStyle::CharacterStyle(const Font& font, const StyleRef& style)
	: fFont(font)
	, fStyle(style)
{
}

// ==
bool
CharacterStyle::operator==(const CharacterStyle& other) const
{
	return fFont == other.fFont && *(fStyle.Get()) == *(other.fStyle.Get());
}

// !=
bool
CharacterStyle::operator!=(const CharacterStyle& other) const
{
	return *this != other;
}
