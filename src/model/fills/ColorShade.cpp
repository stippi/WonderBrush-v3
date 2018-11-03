/*
 * Copyright 2009-2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "ColorShade.h"

#include <algorithm>
#include <stdio.h>

#include <OS.h>

#include "CloneContext.h"
#include "CommonPropertyIDs.h"
#include "Property.h"
#include "Paint.h"
#include "rgb_hsl.h"
#include "ui_defines.h"

// constructor
ColorShade::ColorShade()
	:
	ColorProvider(),
	fColorProvider(),
	fHue(0.0f),
	fSaturation(0.0f),
	fValue(0.0f)
{
}

// constructor
ColorShade::ColorShade(const ColorShade& other, CloneContext& context)
	:
	ColorProvider(),
	fColorProvider(),
	fHue(other.fHue),
	fSaturation(other.fSaturation),
	fValue(other.fValue)
{
	BaseObjectRef ref = context.Clone(other.fColorProvider.Get());
	ColorProvider* provider = dynamic_cast<ColorProvider*>(ref.Get());
	if (provider != NULL)
		SetColorProvider(ColorProviderRef(provider));
}

// constructor
ColorShade::ColorShade(const ColorProviderRef& provider)
	:
	ColorProvider(),
	fColorProvider(),
	fHue(0.0f),
	fSaturation(0.0f),
	fValue(0.0f)
{
	SetColorProvider(provider);
}

// destructor
ColorShade::~ColorShade()
{
	SetColorProvider(ColorProviderRef());
}

// #pragma mark - ColorProvider

// GetColor
rgb_color
ColorShade::GetColor() const
{
	rgb_color color = kBlack;
	if (fColorProvider.Get() != NULL)
		color = fColorProvider->GetColor();

	if (fHue != 0.0f || fSaturation != 0.0f || fValue != 0.0f) {
		float r = color.red / 255.0f;
		float g = color.green / 255.0f;
		float b = color.blue / 255.0f;

		HSL hsl = RGB_to_HSL(r, g, b);

		hsl.h = fmod(hsl.h + fHue, 360.0f);
		hsl.s = std::max(0.0f, std::min(1.0f, hsl.s + fSaturation));
		hsl.l = std::max(0.0f, std::min(1.0f, hsl.l + fValue));

		HSL_to_RGB(hsl, color.red, color.green, color.blue);
	}

	return color;
}

// #pragma mark - BaseObject

// Clone
BaseObject*
ColorShade::Clone(CloneContext& context) const
{
	return new(std::nothrow) ColorShade(*this, context);
}

// AddProperties
void
ColorShade::AddProperties(PropertyObject* object, uint32 flags) const
{
	ColorProvider::AddProperties(object, flags);

	object->AddProperty(new (std::nothrow) FloatProperty(
		PROPERTY_HSV_HUE, fHue, -360.0f, 360.0f));
	object->AddProperty(new (std::nothrow) FloatProperty(
		PROPERTY_SATURATION, fSaturation * 100.0f, -100.0f, 100.0f));
	object->AddProperty(new (std::nothrow) FloatProperty(
		PROPERTY_HSV_VALUE, fValue * 100.0f, -100.0f, 100.0f));
}

// SetToPropertyObject
bool
ColorShade::SetToPropertyObject(const PropertyObject* object, uint32 flags)
{
	AutoNotificationSuspender _(this);

	ColorProvider::SetToPropertyObject(object, flags);

	FloatProperty* hueProperty = dynamic_cast<FloatProperty*>(
		object->FindProperty(PROPERTY_HSV_HUE));
	if (hueProperty != NULL)
		SetHue(hueProperty->Value());

	FloatProperty* saturationProperty = dynamic_cast<FloatProperty*>(
		object->FindProperty(PROPERTY_SATURATION));
	if (saturationProperty != NULL)
		SetSaturation(saturationProperty->Value() / 100.0f);

	FloatProperty* valueProperty = dynamic_cast<FloatProperty*>(
		object->FindProperty(PROPERTY_HSV_VALUE));
	if (valueProperty != NULL)
		SetValue(valueProperty->Value() / 100.0f);

	return HasPendingNotifications();
}

// DefaultName
const char*
ColorShade::DefaultName() const
{
	return "ColorShade";
}

// ObjectChanged
void
ColorShade::ObjectChanged(const Notifier* object)
{
	if (object == fColorProvider.Get())
		Notify();
}


// #pragma mark - ColorShade

ColorShade&
ColorShade::SetColorProvider(const ColorProviderRef& provider)
{
	if (fColorProvider == provider)
		return *this;

	if (fColorProvider.Get() != NULL)
		fColorProvider->RemoveListener(this);

	fColorProvider = provider;

	if (fColorProvider.Get() != NULL)
		fColorProvider->AddListener(this);

	return *this;
}

// SetHue
ColorShade&
ColorShade::SetHue(float hue)
{
	if (fHue != hue) {
		fHue = hue;
		Notify();
	}

	return *this;
}

// SetSaturation
ColorShade&
ColorShade::SetSaturation(float saturation)
{
	if (fSaturation != saturation) {
		fSaturation = saturation;
		Notify();
	}

	return *this;
}

// SetValue
ColorShade&
ColorShade::SetValue(float value)
{
	if (fValue != value) {
		fValue = value;
		Notify();
	}

	return *this;
}




