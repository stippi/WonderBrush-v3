/*
 * Copyright 2012-2013 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "StyleRun.h"

#include "CloneContext.h"

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

// constructor
StyleRun::StyleRun(const StyleRun& other, CloneContext& context)
	: fCharacterStyle()
	, fLength(other.fLength)
{
	// See if the context returns a different Style instance. If yes,
	// we need to point our CharacterStyle ref to our own CharacterStyle,
	// otherwise we can reference the CharacterStyle of the other run.
	CharacterStyle* characterStyle = other.fCharacterStyle.Get();
	if (characterStyle != NULL) {
		BaseObjectRef style = context.Clone(characterStyle->GetStyle().Get());
		if (style.Get() == characterStyle->GetStyle().Get())
			fCharacterStyle = other.fCharacterStyle;
		else {
			fCharacterStyle.SetTo(
				new(std::nothrow) CharacterStyle(
					characterStyle->GetFont(),
					characterStyle->GetGlyphSpacing(),
					characterStyle->GetGlyphSpacing(),
					characterStyle->GetGlyphSpacing(),
					StyleRef(dynamic_cast<Style*>(style.Get()))
				)
			);
		}
	}
}

// =
StyleRun&
StyleRun::operator=(const StyleRun& other)
{
	fCharacterStyle = other.fCharacterStyle;
	fLength = other.fLength;
	return *this;
}

// ==
bool
StyleRun::operator==(const StyleRun& other) const
{
	return fLength == other.fLength && IsSameStyle(other);
}

// !=
bool
StyleRun::operator!=(const StyleRun& other) const
{
	return !(*this == other);
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
