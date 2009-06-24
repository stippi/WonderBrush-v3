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
	Paint("<color>"),
	fColor(kWhite)
{
}

// constructor
PaintColor::PaintColor(const rgb_color& color)
	:
	Paint("<color>"),
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

// SetColor
void
PaintColor::SetColor(const rgb_color& color)
{
	if (*(uint32*)&fColor == *(uint32*)&color)
		return;

	fColor = color;
	Notify();
}

