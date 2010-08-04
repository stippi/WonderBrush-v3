/*
 * Copyright 2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "Style.h"

#include <stdio.h>

#include <new>

#include <Rect.h>

#include "ui_defines.h"

#include "CommonPropertyIDs.h"
#include "PropertyObjectProperty.h"


PaintCache Style::sPaintCache;
StrokePropertiesCache Style::sStrokePropertiesCache;
Style Style::sNullStyle;

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


// constructor
Style::Style()
	:
	BaseObject(),
	fSetProperties(0),
	fFillPaint(NULL),
	fStrokePaint(NULL),
	fStrokeProperties(NULL)
{
	SetFillPaint(Paint::EmptyPaint());
	SetStrokePaint(Paint::EmptyPaint());
	SetStrokeProperties(::StrokeProperties());
}

// constructor
Style::Style(const Style& other)
	:
	BaseObject(other),
	fSetProperties(0),
	fFillPaint(NULL),
	fStrokePaint(NULL),
	fStrokeProperties(NULL)
{
	*this = other;
}

// destructor
Style::~Style()
{
	UnsetFillPaint();
	UnsetStrokePaint();
	UnsetStrokeProperties();
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

// AddProperties
void
Style::AddProperties(PropertyObject* object, uint32 flags) const
{
//	BaseObject::AddProperties(object, flags);

	_AddPaintProperties(fFillPaint, PROPERTY_GROUP_FILL_PAINT,
		PROPERTY_FILL_PAINT_TYPE, object, flags | Paint::FILL_PAINT);
	_AddPaintProperties(fStrokePaint, PROPERTY_GROUP_STROKE_PAINT,
		PROPERTY_STROKE_PAINT_TYPE, object, flags | Paint::STROKE_PAINT);
}

// SetToPropertyObject
bool
Style::SetToPropertyObject(const PropertyObject* object, uint32 flags)
{
	AutoNotificationSuspender _(this);
//	BaseObject::SetToPropertyObject(object, flags);

	// TODO: stroke stuff...
	_SetPaintToPropertyObject(fFillPaint, FILL_PAINT, object,
		flags | Paint::FILL_PAINT);
	_SetPaintToPropertyObject(fStrokePaint, STROKE_PAINT, object,
		flags | Paint::STROKE_PAINT);

	return HasPendingNotifications();
}

// #pragma mark -

// operator=
Style&
Style::operator=(const Style& other)
{
	// We can directly add another reference to the SharedPaints and
	// StrokeProperties.
	fSetProperties = other.fSetProperties;

	if (other.fFillPaint != NULL)
		SetFillPaint(*other.fFillPaint);
	else
		UnsetFillPaint();

	if (other.fStrokePaint != NULL)
		SetStrokePaint(*other.fStrokePaint);
	else
		UnsetStrokePaint();

	if (other.fStrokeProperties != NULL)
		SetStrokeProperties(*other.fStrokeProperties);
	else
		UnsetStrokeProperties();

	return *this;
}

// SetFillPaint
void
Style::SetFillPaint(const Paint& paint)
{
	_SetProperty(fFillPaint, paint, sPaintCache, FILL_PAINT);
}

// UnsetFillPaint
void
Style::UnsetFillPaint()
{
	_UnsetProperty(fFillPaint, sPaintCache, FILL_PAINT);
}

// SetStrokePaint
void
Style::SetStrokePaint(const Paint& paint)
{
	_SetProperty(fStrokePaint, paint, sPaintCache, STROKE_PAINT);
}

// UnsetStrokePaint
void
Style::UnsetStrokePaint()
{
	_UnsetProperty(fStrokePaint, sPaintCache, STROKE_PAINT);
}

// SetStrokeProperties
void
Style::SetStrokeProperties(const ::StrokeProperties& properties)
{
	_SetProperty(fStrokeProperties, properties, sStrokePropertiesCache,
		properties.SetProperties());
}

// UnsetStrokeProperties
void
Style::UnsetStrokeProperties()
{
	uint64 properties = STROKE_WIDTH | STROKE_JOIN_MODE | STROKE_CAP_MODE 
		| STROKE_MITER_LIMIT;
	_UnsetProperty(fStrokeProperties, sStrokePropertiesCache, properties);
}

// ExtendBounds
void
Style::ExtendBounds(BRect& bounds) const
{
	if (fStrokePaint == NULL || fStrokePaint->Type() == Paint::NONE)
		return;

	if (fStrokeProperties == NULL)
		return;

	float maxStrokeWidth = fStrokeProperties->Width();

	if (fStrokeProperties->JoinMode() == MiterJoin
		|| fStrokeProperties->JoinMode() == MiterJoinRevert
		|| fStrokeProperties->JoinMode() == MiterJoinRound) {
		maxStrokeWidth *= fStrokeProperties->MiterLimit();
	}

	bounds.InsetBy(-maxStrokeWidth, -maxStrokeWidth);
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

// _SetProperty
template<typename PropertyType, typename ValueType, typename CacheType>
void
Style::_SetProperty(PropertyType*& member, const ValueType& newValue,
	CacheType& cache, uint64 setProperty)
{
	fSetProperties |= setProperty;

	if (member == NULL) {
		member = cache.Get(newValue);
		Notify();
		return;
	}

	if (*member == newValue)
		return;

	member = cache.PrepareForModifications(member);
	*member = newValue;
	member = cache.CommitModifications(member);
	Notify();
}

// _UnsetProperty
template<typename PropertyType, typename CacheType>
void
Style::_UnsetProperty(PropertyType*& member, CacheType& cache,
	uint64 setProperty)
{
	if (member == NULL)
		return;

	fSetProperties &= ~setProperty;

	cache.Put(member);
	member = NULL;
	Notify();
}

// _AddPaintProperties
void
Style::_AddPaintProperties(const Paint* paint, uint32 groupID,
	uint32 paintTypeID, PropertyObject* object, uint32 flags) const
{
	PropertyObjectProperty* paintProperties
		= new(std::nothrow) PropertyObjectProperty(groupID);
	if (paintProperties == NULL || !object->AddProperty(paintProperties)) {
		delete paintProperties;
		return;
	}

	if (paint != NULL)
		paint->AddProperties(&paintProperties->Value(), flags);
	else {
		Paint::AddTypeProperty(&paintProperties->Value(), paintTypeID,
			Paint::NONE);
	}
}

// _SetPaintToPropertyObject
void
Style::_SetPaintToPropertyObject(SharedPaint*& member, uint64 setProperty,
	const PropertyObject* object, uint32 flags)
{
	Paint paint;
	if (member != NULL)
		paint = *member;

	// We never unset the Paint member, since that would make it hard
	// to revert changing only certain properties of the paint. Like for
	// example when we have a color paint, then set the paint type to none,
	// nobody would store the previous color if we would unset the paint
	// entirely. If we set the type back to color, we would get the default
	// color. Keeping the paint even if its type is 'none' means we can change
	// any single property of the paint while still storing the others.
	if (paint.SetToPropertyObject(object, flags))
		_SetProperty(member, paint, sPaintCache, setProperty);
}

