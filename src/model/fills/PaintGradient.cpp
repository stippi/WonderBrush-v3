/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "PaintGradient.h"

#include <new>

#include <Message.h>

#include "Gradient.h"
#include "ui_defines.h"

// constructor
PaintGradient::PaintGradient()
	:
	Paint(),
	fGradient(NULL),
	fColors(NULL),

	fGammaCorrectedColors(NULL),
	fGammaCorrectedColorsValid(false)
{
}

// constructor
PaintGradient::PaintGradient(const PaintGradient& other)
	:
	Paint(other),
	fGradient(NULL),
	fColors(NULL),

	fGammaCorrectedColors(NULL),
	fGammaCorrectedColorsValid(false)
{
	SetGradient(other.fGradient);
}

// constructor
PaintGradient::PaintGradient(BMessage* archive)
	:
	Paint(archive),
	fGradient(NULL),
	fColors(NULL),

	fGammaCorrectedColors(NULL),
	fGammaCorrectedColorsValid(false)
{
	if (archive == NULL)
		return;

	BMessage gradientArchive;
	if (archive->FindMessage("gradient", &gradientArchive) == B_OK) {
		::Gradient gradient(&gradientArchive);
		SetGradient(&gradient);
	}
}

// destructor
PaintGradient::~PaintGradient()
{
	SetGradient(NULL);
}

// DefaultName
const char*
PaintGradient::DefaultName() const
{
	return "Gradient";
}

// #pragma mark -

// ObjectChanged
void
PaintGradient::ObjectChanged(const Notifier* object)
{
	if (object == fGradient && fColors) {
		fGradient->MakeGradient((uint32*)fColors, 256);
		fGammaCorrectedColorsValid = false;
		Notify();
	}
}

// #pragma mark -

// Archive
status_t
PaintGradient::Archive(BMessage* into, bool deep) const
{
	status_t ret = Paint::Archive(into, deep);

	if (ret == B_OK && fGradient) {
		BMessage gradientArchive;
		ret = fGradient->Archive(&gradientArchive, deep);
		if (ret == B_OK)
			ret = into->AddMessage("gradient", &gradientArchive);
	}

	return ret;
}

// Clone
Paint*
PaintGradient::Clone() const
{
	return new (std::nothrow) PaintGradient(*this);
}

// HasTransparency
bool
PaintGradient::HasTransparency() const
{
	if (fGradient != NULL) {
		int32 count = fGradient->CountColors();
		for (int32 i = 0; i < count; i++) {
			Gradient::ColorStop* stop = fGradient->ColorAtFast(i);
			if (stop->color.alpha < 255)
				return true;
		}
	}
	return false;
}

// SetGradient
void
PaintGradient::SetGradient(const ::Gradient* gradient)
{
	if (fGradient == NULL && gradient == NULL)
		return;

	if (gradient != NULL) {
		if (fGradient == NULL) {
			fGradient = new (nothrow) ::Gradient(*gradient);
			if (fGradient != NULL) {
				fGradient->AddListener(this);
				// generate gradient
				fColors = new agg::rgba8[256];
				fGradient->MakeGradient((uint32*)fColors, 256);
				fGammaCorrectedColorsValid = false;

				Notify();
			}
		} else {
			if (*fGradient != *gradient) {
				*fGradient = *gradient;
				// Notification is triggered in ObjectChanged()
			}
		}
	} else {
		fGradient->RemoveListener(this);
		delete[] fColors;
		delete[] fGammaCorrectedColors;
		fColors = NULL;
		fGammaCorrectedColors = NULL;
		fGradient = NULL;

		Notify();
	}
}

//// GammaCorrectedColors
//const agg::rgba8*
//PaintGradient::GammaCorrectedColors(const GammaTable& table) const
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
