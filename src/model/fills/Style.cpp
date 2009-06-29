/*
 * Copyright 2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "Style.h"

#include <stdio.h>

#include <new>

#include "Paint.h"
#include "PaintColor.h"
#include "SharedObjectCache.h"

#include "ui_defines.h"


typedef SharedObject<PaintColor>			SharedPaintColor;
typedef SharedObjectCache<PaintColor>		PaintColorCache;

static PaintColorCache sPaintColorCache;

static int
test_paint_cache()
{
	PaintColor color1(kBlack);
	PaintColor color2(kBlack);
	PaintColor color3(kWhite);

	SharedPaintColor* sharedColor1 = sPaintColorCache.Get(color1);
	SharedPaintColor* sharedColor2 = sPaintColorCache.Get(color2);
	SharedPaintColor* sharedColor3 = sPaintColorCache.Get(color3);

	printf("sharedColor1 (%p) ref count: %ld\n", sharedColor1, sharedColor1->CountReferences());
	printf("sharedColor2 (%p) ref count: %ld\n", sharedColor2, sharedColor2->CountReferences());
	printf("sharedColor3 (%p) ref count: %ld\n", sharedColor3, sharedColor3->CountReferences());

	SharedPaintColor* modified1
		= sPaintColorCache.PrepareForModifications(sharedColor1);
	printf("modified1 before: %p\n", modified1);

	*modified1 = *sharedColor3;

	modified1 = sPaintColorCache.CommitModifications(modified1);
	printf("modified1 after: %p\n", modified1);

	printf("sharedColor1 (%p) ref count: %ld\n", sharedColor1, sharedColor1->CountReferences());
	printf("sharedColor2 (%p) ref count: %ld\n", sharedColor2, sharedColor2->CountReferences());
	printf("sharedColor3 (%p) ref count: %ld\n", sharedColor3, sharedColor3->CountReferences());

	return 0;
}

static int test = test_paint_cache();


// constructor
Style::Style()
	:
	BaseObject(),
	fSetProperties(0),
	fFillPaint(NULL),
	fStrokePaint(NULL),
	fStrokeProperties(NULL)
{
}

// constructor
Style::Style(const Style& other)
	:
	BaseObject(other),
	fSetProperties(other.fSetProperties),
	fFillPaint(NULL),
	fStrokePaint(NULL),
	fStrokeProperties(NULL)
{
	SetFillPaint(other.fFillPaint);
	SetStrokePaint(other.fStrokePaint);
	SetStrokeProperties(other.fStrokeProperties);
}

// destructor
Style::~Style()
{
}

// #pragma mark - BaseObject

// Unarchive
status_t
Style::Unarchive(const BMessage* archive)
{
	status_t ret = BaseObject::Unarchive(archive);

	// TODO ...

	return ret;
}

// Archive
status_t
Style::Archive(BMessage* into, bool deep) const
{
	status_t ret = BaseObject::Archive(into, deep);

	// TODO ...

	return ret;
}


// DefaultName
const char*
Style::DefaultName() const
{
	return "Style";
}

// MakePropertyObject
PropertyObject*
Style::MakePropertyObject() const
{
	PropertyObject* object = BaseObject::MakePropertyObject();
	if (object == NULL)
		return NULL;

//	object->AddProperty(new (std::nothrow) FloatProperty(
//		PROPERTY_FILTER_RADIUS, fFilterRadius, 0.0f, 10000.0f));

	return object;
}

// SetToPropertyObject
bool
Style::SetToPropertyObject(const PropertyObject* object)
{
	AutoNotificationSuspender _(this);
	BaseObject::SetToPropertyObject(object);

//	SetFilterRadius(object->Value(PROPERTY_FILTER_RADIUS, fFilterRadius));

	return HasPendingNotifications();
}

// SetFillPaint
void
Style::SetFillPaint(Paint* paint)
{
	_SetProperty(fFillPaint, paint, FILL_PAINT);
}

// SetStrokePaint
void
Style::SetStrokePaint(Paint* paint)
{
	_SetProperty(fStrokePaint, paint, STROKE_PAINT);
}

// SetStrokeProperties
void
Style::SetStrokeProperties(const ::StrokeProperties& properties)
{
	// TODO
}

// SetStrokeProperties
void
Style::SetStrokeProperties(::StrokeProperties* properties)
{
	uint64 setProperties = 0;
	if (properties != NULL)
		setProperties = properties->SetProperties();
	_SetProperty(fStrokeProperties, properties, setProperties);
}

// #pragma mark -

// _SetProperty
template<typename PropertyType>
void
Style::_SetProperty(PropertyType*& member, PropertyType* newMember,
	uint64 setProperty)
{
	if (member == newMember)
		return;

	if (member != NULL)
		member->RemoveReference();

	member = newMember;

	if (member != NULL) {
		member->AddReference();
		fSetProperties |= setProperty;
	} else {
		fSetProperties &= ~setProperty;
	}

	Notify();
}

