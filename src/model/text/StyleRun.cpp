/*
 * Copyright 2012 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "StyleRun.h"

// constructor
StyleRun::StyleRun(const CharacterStyleRef& characterStyle)
	: fCharacterStyle(characterStyle)
	, fLength(0)
{
}

// constructor
StyleRun::StyleRun(const StyleRun& other)
	: fCharacterStyle(other.fCharacterStyle)
	, fLength(other.fLength)
{
}

// SetLength
void
StyleRun::SetLength(int32 length)
{
	fLength = length;
}

// IsSameStyle
bool
StyleRun::IsSameStyle(const StyleRun& other) const
{
	return *fCharacterStyle.Get() == *other.fCharacterStyle.Get();
}
