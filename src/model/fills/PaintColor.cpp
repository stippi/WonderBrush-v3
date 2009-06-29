/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "PaintColor.h"

#include <Message.h>

#include <new>

#include "ui_defines.h"

// constructor
PaintColor::PaintColor()
	:
	Paint(),
	fColor(kWhite)
{
}

// constructor
PaintColor::PaintColor(const rgb_color& color)
	:
	Paint(),
	fColor(color)
{
}

// constructor
PaintColor::PaintColor(const PaintColor& other)
	:
	Paint(other),
	fColor(other.fColor)
{
}

// constructor
PaintColor::PaintColor(BMessage* archive)
	:
	Paint(archive),
	fColor(kWhite)
{
	if (archive == NULL)
		return;

	if (archive->FindInt32("color", (int32*)&fColor) < B_OK)
		fColor = kWhite;
}

// destructor
PaintColor::~PaintColor()
{
}

// #pragma mark -

// DefaultName
const char*
PaintColor::DefaultName() const
{
	return "Color";
}

// #pragma mark -

// Archive
status_t
PaintColor::Archive(BMessage* into, bool deep) const
{
	status_t ret = Paint::Archive(into, deep);

	if (ret == B_OK)
		ret = into->AddInt32("color", (uint32&)fColor);

	return ret;
}

// Clone
Paint*
PaintColor::Clone() const
{
	return new (std::nothrow) PaintColor(*this);
}

// HasTransparency
bool
PaintColor::HasTransparency() const
{
	return fColor.alpha < 255;
}

// HashKey
size_t
PaintColor::HashKey() const
{
	// TODO: Shrink, maybe 6 bits per component?
	return (size_t&)fColor;
}

// operator==
bool
PaintColor::operator==(const PaintColor& other) const
{
	return other.fColor == fColor;
}

// SetColor
void
PaintColor::SetColor(const rgb_color& color)
{
	if (*(uint32*)&fColor == *(uint32*)&color)
		return;

	fColor = color;
	Notify();
}

