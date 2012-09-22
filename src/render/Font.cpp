/*
 * Copyright 2012 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */
#include "Font.h"

#include <stdio.h>
#include <string.h>


Font::Font(const char* name, double size, unsigned style)
	:
	fSize(size),
	fStyle(style),
	fScriptLevel(NORMAL)
{
	snprintf(fName, sizeof(fName), "%s", name);
}


Font::Font(const char* name, double size, unsigned style,
		ScriptLevel scriptLevel)
	:
	fSize(size),
	fStyle(style),
	fScriptLevel(scriptLevel)
{
	snprintf(fName, sizeof(fName), "%s", name);
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
	return fSize == other.fSize && fStyle == other.fStyle
		&& fScriptLevel == other.fScriptLevel
		&& strcmp(fName, other.fName) == 0;
}


bool
Font::operator!=(const Font& other) const
{
	return !(*this == other);
}


Font&
Font::operator=(const Font& other)
{
	memcpy(fName, other.fName, sizeof(fName));
	fSize = other.fSize;
	fStyle = other.fStyle;
	fScriptLevel = other.fScriptLevel;

	return *this;
}

