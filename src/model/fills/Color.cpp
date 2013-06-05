/*
 * Copyright 2009-2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "Color.h"

#include <stdio.h>

#include <OS.h>

#include "ui_defines.h"

// constructor
Color::Color()
	:
	ColorProvider(),
	fColor(kBlack)
{
}

// constructor
Color::Color(const Color& color)
	:
	ColorProvider(),
	fColor(color.fColor)
{
}

// constructor
Color::Color(const rgb_color& color)
	:
	ColorProvider(),
	fColor(color)
{
}

// destructor
Color::~Color()
{
}

// GetColor
rgb_color
Color::GetColor() const
{
	return fColor;
}

// DefaultName
const char*
Color::DefaultName() const
{
	return "Color";
}

// SetColor
Color&
Color::SetColor(const rgb_color& color)
{
	if ((uint32&)fColor != (uint32&)color) {
		fColor = color;
		Notify();
	}

	return *this;
}

// operator=
Color&
Color::operator=(const rgb_color& color)
{
	return SetColor(color);
}

// operator=
Color&
Color::operator=(const Color& color)
{
	return SetColor(color.fColor);
}

// operator==
bool
Color::operator==(const rgb_color& color) const
{
	return (uint32&)fColor == (uint32&)color;
}

// operator!=
bool
Color::operator!=(const rgb_color& color) const
{
	return (uint32&)fColor != (uint32&)color;
}

// operator==
bool
Color::operator==(const Color& color) const
{
	return (uint32&)fColor == (uint32&)color.fColor;
}

// operator!=
bool
Color::operator!=(const Color& color) const
{
	return (uint32&)fColor != (uint32&)color.fColor;
}




