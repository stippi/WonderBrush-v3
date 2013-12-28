/*
 * Copyright 2006-2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "Paint.h"

#include <new>

#include <stdio.h>

#include <Message.h>

#include "CloneContext.h"
#include "Color.h"
#include "ColorProperty.h"
#include "Gradient.h"
#include "OptionProperty.h"
#include "ui_defines.h"

// constructor
Paint::Paint()
	: BaseObject()
	, fType(NONE)
	, fColor(new(std::nothrow) ::Color())
	, fGradient(NULL)

	, fColors(NULL)

	, fGammaCorrectedColors(NULL)
	, fGammaCorrectedColorsValid(false)
{
}

// constructor
Paint::Paint(const Paint& other, CloneContext& context)
	: BaseObject(other)
	, fType(NONE)
	, fColor(other.fColor)
	, fGradient(NULL)

	, fColors(NULL)

	, fGammaCorrectedColors(NULL)
	, fGammaCorrectedColorsValid(false)
{
	*this = other;

	context.Clone(other.fColor.Get(), fColor);
}

// constructor
Paint::Paint(const rgb_color& color)
	: BaseObject()
	, fType(COLOR)
	, fColor(new(std::nothrow) ::Color(color))
	, fGradient(NULL)

	, fColors(NULL)

	, fGammaCorrectedColors(NULL)
	, fGammaCorrectedColorsValid(false)
{
}

// constructor
Paint::Paint(const ColorProviderRef& color)
	: BaseObject()
	, fType(COLOR)
	, fColor(color)
	, fGradient(NULL)

	, fColors(NULL)

	, fGammaCorrectedColors(NULL)
	, fGammaCorrectedColorsValid(false)
{
}

// constructor
Paint::Paint(const ::Gradient* gradient)
	: BaseObject()
	, fType(GRADIENT)
	, fColor(new(std::nothrow) ::Color())
	, fGradient(NULL)

	, fColors(NULL)

	, fGammaCorrectedColors(NULL)
	, fGammaCorrectedColorsValid(false)
{
	SetGradient(gradient);
}

// constructor
Paint::Paint(BMessage* archive)
	: BaseObject()
	, fType(NONE)
	, fColor(new(std::nothrow) ::Color())
	, fGradient(NULL)

	, fColors(NULL)

	, fGammaCorrectedColors(NULL)
	, fGammaCorrectedColorsValid(false)
{
	Unarchive(archive);
}

// destructor
Paint::~Paint()
{
	SetGradient(NULL);
	// TODO: pattern...
}

// #pragma mark -

// Clone
BaseObject*
Paint::Clone(CloneContext& context) const
{
	return new(std::nothrow) Paint(*this, context);
}

// Unarchive
status_t
Paint::Unarchive(const BMessage* archive)
{
	status_t ret = BaseObject::Unarchive(archive);

	// TODO: ...
//	if (archive == NULL)
//		return B_BAD_VALUE;
//
//	if (archive->FindInt32("color", (int32*)&fColor) < B_OK)
//		fColor = kWhite;

//	BMessage gradientArchive;
//	if (archive->FindMessage("gradient", &gradientArchive) == B_OK) {
//		::Gradient gradient(&gradientArchive);
//		SetGradient(&gradient);
//	}

	return ret;
}

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
			{
				BMessage colorArchive;
				ret = fColor->Archive(&colorArchive, deep);
				if (ret == B_OK)
					ret = into->AddMessage("color", &colorArchive);
				break;
			}

			case GRADIENT:
			{
				BMessage gradientArchive;
				ret = fGradient->Archive(&gradientArchive, deep);
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

// AddProperties
void
Paint::AddProperties(PropertyObject* object, uint32 flags) const
{
	uint32 typeID = PROPERTY_FILL_PAINT_TYPE;
	if ((flags & STROKE_PAINT) != 0)
		typeID = PROPERTY_STROKE_PAINT_TYPE;

	AddTypeProperty(object, typeID, fType);

	switch (fType) {
		default:
		case NONE:
			break;
		case COLOR:
			if (fColor.Get() != NULL) {
				fColor->AddProperties(object,
					flags | BaseObject::DONT_ADD_NAME);
			}
			break;
		case GRADIENT:
			// TODO: ...
			break;
		case PATTERN:
			// TODO: ...
			break;
	}
}

// SetToPropertyObject
bool
Paint::SetToPropertyObject(const PropertyObject* object, uint32 flags)
{
	AutoNotificationSuspender _(this);

	BaseObject::SetToPropertyObject(object, flags);

	uint32 typeID = PROPERTY_FILL_PAINT_TYPE;
	if ((flags & STROKE_PAINT) != 0)
		typeID = PROPERTY_STROKE_PAINT_TYPE;

	// Adopt the type first
	OptionProperty* typeProperty = dynamic_cast<OptionProperty*>(
		object->FindProperty(typeID));
	if (typeProperty != NULL)
		SetType(typeProperty->CurrentOptionID());

	// Adopt rest of properties according to type
	// NOTE: SetToPropertyObject() will usually be called as soon as
	// one property has been changed. So the type should change just
	// by itself. In any case, the properties we find in the object
	// should not mismatch. But no harm if they do.
	switch (fType) {
		default:
		case NONE:
			break;
		case COLOR:
			if (fColor.Get() != NULL) {
				if (fColor->SetToPropertyObject(object,
						flags | BaseObject::DONT_ADD_NAME)) {
					Notify();
				}
			}
			break;
		case GRADIENT:
			// TODO: ...
			break;
		case PATTERN:
			// TODO: ...
			break;
	}

	return HasPendingNotifications();
}

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
	if (object == fGradient && fColors != NULL) {
		fGradient->MakeGradient((uint32*)fColors, 256);
		fGammaCorrectedColorsValid = false;
		Notify();
	}
}

// #pragma mark -

// operator=
Paint&
Paint::operator=(const Paint& other)
{
	if (&other == this)
		return *this;

	fType = other.fType;
	fColor = other.fColor;
	SetGradient(other.fGradient);
	// TODO: pattern...

	return *this;
}

// operator==
bool
Paint::operator==(const Paint& other) const
{
	return fType == other.fType && fColor == other.fColor
		&& fGradient == other.fGradient;
		// TODO: pattern...
}

// operator!=
bool
Paint::operator!=(const Paint& other) const
{
	return !(*this == other);
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
			return fColor->GetColor().alpha < 255;

		case GRADIENT:
			if (fGradient != NULL) {
				int32 count = fGradient->CountColors();
				for (int32 i = 0; i < count; i++) {
					Gradient::ColorStop* stop = fGradient->ColorAtFast(i);
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
	size_t hash = 0;
	hash |= fType << 30;
	rgb_color color = fColor->GetColor();
	hash |= (color.red & 0x0f) << 26;
	hash |= (color.green & 0x0f) << 22;
	hash |= (color.blue & 0x0f) << 18;
	hash |= (color.alpha & 0x0f) << 14;
	hash |= ((addr_t)fGradient & 0xff) << 6;

	// TODO: pattern...

	return hash;
}

// #pragma mark -

// SetType
void
Paint::SetType(uint32 type)
{
	if (fType == type)
		return;

	fType = type;
	Notify();
}

// AddTypeProperty
/*static*/ void
Paint::AddTypeProperty(PropertyObject* object, uint32 propertyID, uint32 type)
{
	OptionProperty* typeProperty = new OptionProperty(propertyID);
	typeProperty->AddOption(NONE, "None");
	typeProperty->AddOption(COLOR, "Color");
	typeProperty->AddOption(GRADIENT, "Gradient");
	typeProperty->AddOption(PATTERN, "Pattern");
	typeProperty->SetCurrentOptionID(type);
	object->AddProperty(typeProperty);
}

// SetColor
void
Paint::SetColor(const rgb_color& color)
{
	if (fType != COLOR)
		fType = COLOR;
	else {
		if (fColor->GetColor() == color)
			return;
	}

	fColor.SetTo(new(std::nothrow) ::Color(color));
	Notify();
}

// SetGradient
void
Paint::SetGradient(const ::Gradient* gradient)
{
	if (gradient != NULL && fType != GRADIENT)
		fType = GRADIENT;
	else {
		if (fGradient == NULL && gradient == NULL)
			return;
	}

	if (gradient != NULL) {
		if (fGradient == NULL) {
			fGradient = new (std::nothrow) ::Gradient(*gradient);
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


//static int
//test_paint_cache()
//{
//	Paint color1(kBlack);
//	Paint color2(kBlack);
//	Paint color3(kWhite);
//
//	SharedPaint* sharedColor1 = sPaintCache.Get(color1);
//	SharedPaint* sharedColor2 = sPaintCache.Get(color2);
//	SharedPaint* sharedColor3 = sPaintCache.Get(color3);
//
//	printf("sharedColor1 (%p) ref count: %ld\n", sharedColor1, sharedColor1->CountReferences());
//	printf("sharedColor2 (%p) ref count: %ld\n", sharedColor2, sharedColor2->CountReferences());
//	printf("sharedColor3 (%p) ref count: %ld\n", sharedColor3, sharedColor3->CountReferences());
//
//	SharedPaint* modified1
//		= sPaintCache.PrepareForModifications(sharedColor1);
//	printf("modified1 before: %p\n", modified1);
//
//	*modified1 = *sharedColor3;
//
//	modified1 = sPaintCache.CommitModifications(modified1);
//	printf("modified1 after: %p\n", modified1);
//
//	printf("sharedColor1 (%p) ref count: %ld\n", sharedColor1, sharedColor1->CountReferences());
//	printf("sharedColor2 (%p) ref count: %ld\n", sharedColor2, sharedColor2->CountReferences());
//	printf("sharedColor3 (%p) ref count: %ld\n", sharedColor3, sharedColor3->CountReferences());
//
//	return 0;
//}
//
//static int test = test_paint_cache();

/*static*/ const Paint&
Paint::EmptyPaint()
{
	static Paint emptyPaint;
	return emptyPaint;
}

/*static*/ ::PaintCache&
Paint::PaintCache()
{
	static ::PaintCache paintCache;
	return paintCache;
}
