/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "Paint.h"

#include <new>

#include <Message.h>

#include "Gradient.h"
#include "ui_defines.h"

// constructor
Paint::Paint()
	:
	BaseObject(),
	fType(NONE),
	fColors(NULL),

	fGammaCorrectedColors(NULL),
	fGammaCorrectedColorsValid(false)
{
	fData.gradient = NULL;
}

// constructor
Paint::Paint(const Paint& other)
	:
	BaseObject(other),
	fType(NONE),
	fColors(NULL),

	fGammaCorrectedColors(NULL),
	fGammaCorrectedColorsValid(false)
{
	fData.gradient = NULL;

	*this = other;
}

// constructor
Paint::Paint(const rgb_color& color)
	:
	BaseObject(),
	fType(COLOR),
	fColors(NULL),

	fGammaCorrectedColors(NULL),
	fGammaCorrectedColorsValid(false)
{
	(rgb_color&)fData.color = color;
}

// constructor
Paint::Paint(const ::Gradient* gradient)
	:
	BaseObject(),
	fType(GRADIENT),
	fColors(NULL),

	fGammaCorrectedColors(NULL),
	fGammaCorrectedColorsValid(false)
{
	fData.gradient = NULL;
	SetGradient(gradient);
}

// constructor
Paint::Paint(BMessage* archive)
	:
	BaseObject(archive),
	fType(NONE),
	fColors(NULL),

	fGammaCorrectedColors(NULL),
	fGammaCorrectedColorsValid(false)
{
	// TODO: ...
//	if (archive == NULL)
//		return;
//
//	if (archive->FindInt32("color", (int32*)&fColor) < B_OK)
//		fColor = kWhite;

//	BMessage gradientArchive;
//	if (archive->FindMessage("gradient", &gradientArchive) == B_OK) {
//		::Gradient gradient(&gradientArchive);
//		SetGradient(&gradient);
//	}

}

// destructor
Paint::~Paint()
{
	if (fType == GRADIENT)
		SetGradient(NULL);
}

// #pragma mark -

// DefaultName
const char*
Paint::DefaultName() const
{
	switch (fType) {
		default:
		case NONE:
			return "No Paint";

		case COLOR:
			return "Color";

		case GRADIENT:
			return "Gradient";

		case PATTERN:
			return "Pattern";
	}
}

// #pragma mark -

// ObjectChanged
void
Paint::ObjectChanged(const Notifier* object)
{
	if (object == fData.gradient && fColors) {
		fData.gradient->MakeGradient((uint32*)fColors, 256);
		fGammaCorrectedColorsValid = false;
		Notify();
	}
}

// #pragma mark -

// Archive
status_t
Paint::Archive(BMessage* into, bool deep) const
{
	status_t ret = BaseObject::Archive(into, deep);

	if (ret == B_OK)
		ret = into->AddInt32("type", fType);

	if (ret == B_OK) {
		switch (fType) {
			default:
			case NONE:
				break;

			case COLOR:
				ret = into->AddInt32("color", fData.color);
				break;

			case GRADIENT:
			{
				BMessage gradientArchive;
				ret = fData.gradient->Archive(&gradientArchive, deep);
				if (ret == B_OK)
					ret = into->AddMessage("gradient", &gradientArchive);
				break;
			}

			case PATTERN:
				// TODO: ...
				break;
		}
	}

	return ret;
}

// Unset
void
Paint::Unset()
{
	switch (fType) {
		default:
		case NONE:
			break;
		case GRADIENT:
			SetGradient(NULL);
			break;
		case PATTERN:
			// TODO: ...
			break;
	}

	fType = NONE;
}

// operator=
Paint&
Paint::operator=(const Paint& other)
{
	if (&other == this)
		return *this;

	Unset();

	fType = other.fType;

	switch (fType) {
		default:
		case NONE:
			break;

		case COLOR:
			fData.color = other.fData.color;
			break;

		case GRADIENT:
			SetGradient(other.fData.gradient);
			break;

		case PATTERN:
			// TODO: ...
			break;
	}

	return *this;
}

// operator==
bool
Paint::operator==(const Paint& other) const
{
	if (fType != other.fType)
		return false;
	switch (fType) {
		default:
		case NONE:
			return true;

		case COLOR:
			return other.fData.color == fData.color;

		case GRADIENT:
			if (fData.gradient == NULL) {
				if (other.fData.gradient == NULL)
					return true;
				return false;
			} else {
				if (other.fData.gradient == NULL)
					return false;
				return *other.fData.gradient == *fData.gradient;
			}

		case PATTERN:
			// TODO: ...
			return true;
	}
}

// operator!=
bool
Paint::operator!=(const Paint& other) const
{
	return !(*this == other);
}

// Clone
Paint*
Paint::Clone() const
{
	return new (std::nothrow) Paint(*this);
}

// HasTransparency
bool
Paint::HasTransparency() const
{
	switch (fType) {
		default:
		case NONE:
			return false;

		case COLOR:
			return ((rgb_color&)fData.color).alpha < 255;

		case GRADIENT:
			if (fData.gradient != NULL) {
				int32 count = fData.gradient->CountColors();
				for (int32 i = 0; i < count; i++) {
					Gradient::ColorStop* stop = fData.gradient->ColorAtFast(i);
					if (stop->color.alpha < 255)
						return true;
				}
			}
			return false;

		case PATTERN:
			// TODO: ...
			return false;
	}
}

// HashKey
size_t
Paint::HashKey() const
{
	// TODO: Shrink, maybe 6 bits per component?
	return (size_t&)fData.color;
}

// #pragma mark -

// SetColor
void
Paint::SetColor(const rgb_color& color)
{
	if (fType != COLOR) {
		Unset();
		fType = COLOR;
	} else {
		if (fData.color == *(uint32*)&color)
			return;
	}

	fData.color = (uint32&)color;
	Notify();
}

// SetGradient
void
Paint::SetGradient(const ::Gradient* gradient)
{
	if (fType != GRADIENT) {
		Unset();
		fType = GRADIENT;
	} else {
		if (fData.gradient == NULL && gradient == NULL)
			return;
	}

	if (gradient != NULL) {
		if (fData.gradient == NULL) {
			fData.gradient = new (nothrow) ::Gradient(*gradient);
			if (fData.gradient != NULL) {
				fData.gradient->AddListener(this);
				// generate gradient
				fColors = new agg::rgba8[256];
				fData.gradient->MakeGradient((uint32*)fColors, 256);
				fGammaCorrectedColorsValid = false;

				Notify();
			}
		} else {
			if (*fData.gradient != *gradient) {
				*fData.gradient = *gradient;
				// Notification is triggered in ObjectChanged()
			}
		}
	} else {
		fData.gradient->RemoveListener(this);
		delete[] fColors;
		delete[] fGammaCorrectedColors;
		fColors = NULL;
		fGammaCorrectedColors = NULL;
		fData.gradient = NULL;

		Notify();
	}
}

//// GammaCorrectedColors
//const agg::rgba8*
//Paint::GammaCorrectedColors(const GammaTable& table) const
//{
//	if (!fColors)
//		return NULL;
//
//	if (!fGammaCorrectedColors)
//		fGammaCorrectedColors = new agg::rgba8[256];
//
//	if (!fGammaCorrectedColorsValid) {
//		for (int32 i = 0; i < 256; i++) {
//			fGammaCorrectedColors[i].r = table.dir(fColors[i].r);
//			fGammaCorrectedColors[i].g = table.dir(fColors[i].g);
//			fGammaCorrectedColors[i].b = table.dir(fColors[i].b);
//			fGammaCorrectedColors[i].a = fColors[i].a;
//			fGammaCorrectedColors[i].premultiply();
//		}
//		fGammaCorrectedColorsValid = true;
//	}
//
//	return fGammaCorrectedColors;
//}



