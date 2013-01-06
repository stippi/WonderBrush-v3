/*
 * Copyright 2012 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */
#include "Font.h"

#include <stdio.h>
#include <string.h>

#include "FontRegistry.h"


Font::Font(const char* family, double size)
	: fFamily(family)
	, fSize(size)
	, fScriptLevel(NORMAL)
{
}


Font::Font(const char* family, const char* style, double size)
	: fFamily(family)
	, fStyle(style)
	, fSize(size)
	, fScriptLevel(NORMAL)
{
}


Font::Font(const char* family, const char* style, double size,
		ScriptLevel scriptLevel)
	: fFamily(family)
	, fStyle(style)
	, fSize(size)
	, fScriptLevel(scriptLevel)
{
}


Font::Font(const Font& other)
{
	*this = other;
}


Font::~Font()
{
}


bool
Font::operator==(const Font& other) const
{
	return fSize == other.fSize && fScriptLevel == other.fScriptLevel
		&& fFamily == other.fFamily && fStyle == other.fStyle;
}


bool
Font::operator!=(const Font& other) const
{
	return !(*this == other);
}


Font&
Font::operator=(const Font& other)
{
	fFamily = other.fFamily;
	fStyle = other.fStyle;
	fFontFilePath = other.fFontFilePath;
	fSize = other.fSize;
	fScriptLevel = other.fScriptLevel;

	return *this;
}


const char*
Font::getFontFilePath() const
{
	if (fFontFilePath.Length() > 0)
		return fFontFilePath.String();

	FontRegistry* registry = FontRegistry::Default();
	if (registry->Lock()) {
		fFontFilePath = registry->FontFileFor(fFamily, fStyle);
		if (fFontFilePath.Length() == 0) {
			fprintf(stderr, "Error matching font '%s'/'%s'\n",
				fFamily.String(), fStyle.String());
		}
		registry->Unlock();
	}

	return fFontFilePath.String();
}


Font
Font::setFamilyAndStyle(const char* family, const char* style) const
{
	Font font(*this);
	font.fFamily = family;
	font.fStyle = style;
	return font;
}


Font
Font::setSize(double size) const
{
	Font font(*this);
	font.fSize = size;
	return font;
}


Font
Font::setScriptLevel(ScriptLevel scriptLevel) const
{
	Font font(*this);
	font.fScriptLevel = scriptLevel;
	return font;
}

