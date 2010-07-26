/*
 * Copyright 2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "Style.h"

#include <stdio.h>

#include <new>

#include "ui_defines.h"

#include "CommonPropertyIDs.h"
#include "PropertyObjectProperty.h"


PaintCache Style::sPaintCache;
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
	// We can directly add another reference to
	if (other.fFillPaint != NULL)
		SetFillPaint(*other.fFillPaint);

	if (other.fStrokePaint != NULL)
		SetFillPaint(*other.fStrokePaint);

	SetStrokeProperties(other.fStrokeProperties);
}

// destructor
Style::~Style()
{
	UnsetFillPaint();
	UnsetStrokePaint();
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

	// TODO: ...
	_SetPaintToPropertyObject(fFillPaint, FILL_PAINT, object,
		flags | Paint::FILL_PAINT);
	_SetPaintToPropertyObject(fStrokePaint, STROKE_PAINT, object,
		flags | Paint::STROKE_PAINT);

	return HasPendingNotifications();
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
	uint64 setProperties = properties.SetProperties();
	if (setProperties == 0) {
		::StrokeProperties* heapProperties
			= new(std::nothrow) ::StrokeProperties(properties);
		_SetProperty(fStrokeProperties, heapProperties, setProperties);
		// Get rid of the creator reference.
		heapProperties->RemoveReference();
	} else
		_SetProperty< ::StrokeProperties>(fStrokeProperties, NULL, 0);
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

	if (paint.SetToPropertyObject(object, flags)) {
		if (paint.Type() == Paint::NONE)
			_UnsetProperty(member, sPaintCache, setProperty);
		else
			_SetProperty(member, paint, sPaintCache, setProperty);
	}
}

