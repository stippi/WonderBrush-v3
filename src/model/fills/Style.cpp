/*
 * Copyright 2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "Style.h"

#include <new>

#include "Paint.h"
#include "PaintColor.h"
#include "SharedObjectCache.h"


#if 0
class SharedPaintColor : public SharedObject<SharedPaintColor>,
	public PaintColor {
public:
	typedef PaintColor KeyType;

	SharedPaintColor()
		:
		PaintColor()
	{
	}

	SharedPaintColor(const PaintColor& color)
		:
		PaintColor(color)
	{
	}
};
#else
typedef SharedObject<Paint> SharedPaint;
#endif

typedef SharedObjectCache<SharedPaint>	PaintCache;

static PaintCache sPaintColorCache;


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

