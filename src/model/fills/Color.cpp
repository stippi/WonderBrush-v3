/*
 * Copyright 2009-2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "Color.h"

#include <stdio.h>

#include <OS.h>

#include "ColorProperty.h"
#include "CommonPropertyIDs.h"
#include "Paint.h"
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

// #pragma mark - ColorProvider

// GetColor
rgb_color
Color::GetColor() const
{
	return fColor;
}

// #pragma mark - BaseObject

// AddProperties
void
Color::AddProperties(PropertyObject* object, uint32 flags) const
{
	ColorProvider::AddProperties(object, flags);

	uint32 colorID = PROPERTY_FILL_PAINT_COLOR;
	if ((flags & Paint::STROKE_PAINT) != 0)
		colorID = PROPERTY_STROKE_PAINT_COLOR;

	object->AddProperty(new (std::nothrow) ColorProperty(colorID, GetColor()));
}

// SetToPropertyObject
bool
Color::SetToPropertyObject(const PropertyObject* object, uint32 flags)
{
	AutoNotificationSuspender _(this);

	ColorProvider::SetToPropertyObject(object, flags);

	uint32 colorID = PROPERTY_FILL_PAINT_COLOR;
	if ((flags & Paint::STROKE_PAINT) != 0)
		colorID = PROPERTY_STROKE_PAINT_COLOR;

	ColorProperty* colorProperty = dynamic_cast<ColorProperty*>(
		object->FindProperty(colorID));
	if (colorProperty != NULL)
		SetColor(colorProperty->Value());

	return HasPendingNotifications();
}

// DefaultName
const char*
Color::DefaultName() const
{
	return "Color";
}

// #pragma mark - Color

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




