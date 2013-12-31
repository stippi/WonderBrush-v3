/*
 * Copyright 2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "Style.h"

#include <stdio.h>

#include <new>

#include <Rect.h>

#include "ui_defines.h"

#include "CloneContext.h"
#include "CommonPropertyIDs.h"
#include "PropertyObjectProperty.h"


// constructor
Style::Style()
	:
	BaseObject(),
	fFillPaint(NULL),
	fStrokePaint(NULL),
	fStrokeProperties(NULL)
{
	SetFillPaint(
		PaintRef(new(std::nothrow) Paint(kBlack), true));
	SetStrokePaint(
		PaintRef(new(std::nothrow) Paint(), true));
	SetStrokeProperties(
		StrokePropertiesRef(new(std::nothrow) ::StrokeProperties(1.0f), true));
}

// constructor
Style::Style(const Style& other, CloneContext& context)
	:
	BaseObject(other),
	fFillPaint(NULL),
	fStrokePaint(NULL),
	fStrokeProperties(NULL)
{
	context.Clone(other.fFillPaint.Get(), fFillPaint);
	context.Clone(other.fStrokePaint.Get(), fStrokePaint);
	context.Clone(other.fStrokeProperties.Get(), fStrokeProperties);

	if (fFillPaint.Get() != NULL)
		fFillPaint->AddListener(this);
	if (fStrokePaint.Get() != NULL)
		fStrokePaint->AddListener(this);
	if (fStrokeProperties.Get() != NULL)
		fStrokeProperties->AddListener(this);
}

// destructor
Style::~Style()
{
	// Remove listeners
	SetFillPaint(PaintRef(NULL));
	SetStrokePaint(PaintRef(NULL));
	SetStrokeProperties(StrokePropertiesRef(NULL));
}

// #pragma mark - BaseObject

// Clone
BaseObject*
Style::Clone(CloneContext& context) const
{
	return new(std::nothrow) Style(*this, context);
}

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

	_AddPaintProperties(fFillPaint.Get(), PROPERTY_GROUP_FILL_PAINT,
		PROPERTY_FILL_PAINT_TYPE, object, flags | Paint::FILL_PAINT);
	_AddPaintProperties(fStrokePaint.Get(), PROPERTY_GROUP_STROKE_PAINT,
		PROPERTY_STROKE_PAINT_TYPE, object, flags | Paint::STROKE_PAINT);
}

// SetToPropertyObject
bool
Style::SetToPropertyObject(const PropertyObject* object, uint32 flags)
{
	AutoNotificationSuspender _(this);
//	BaseObject::SetToPropertyObject(object, flags);

	fFillPaint->SetToPropertyObject(object, flags | Paint::FILL_PAINT);
	fStrokePaint->SetToPropertyObject(object, flags | Paint::STROKE_PAINT);
	fStrokeProperties->SetToPropertyObject(object, flags);

	return HasPendingNotifications();
}

// #pragma mark -

// ObjectChanged
void
Style::ObjectChanged(const Notifier* object)
{
	if (object == fFillPaint.Get() || object == fStrokePaint.Get()
		|| object == fStrokeProperties.Get()) {
		Notify();
	}
}

// #pragma mark -

// operator==
bool
Style::operator==(const Style& other) const
{
	if (this == &other)
		return true;
	return fFillPaint == other.fFillPaint
		&& fStrokePaint == other.fStrokePaint
		&& fStrokeProperties == other.fStrokeProperties;
}

// operator!=
bool
Style::operator!=(const Style& other) const
{
	return !(*this == other);
}

// operator=
Style&
Style::operator=(const Style& other)
{
	// We can directly add another reference to the SharedPaints and
	// StrokeProperties.
	SetFillPaint(other.fFillPaint);
	SetStrokePaint(other.fStrokePaint);
	SetStrokeProperties(other.fStrokeProperties);

	return *this;
}

// SetFillPaint
void
Style::SetFillPaint(const PaintRef& paint)
{
	if (fFillPaint != paint) {
		if (fFillPaint.Get() != NULL)
			fFillPaint->RemoveListener(this);
		
		fFillPaint = paint;

		if (fFillPaint.Get() != NULL)
			fFillPaint->AddListener(this);
		Notify();
	}
}

// SetStrokePaint
void
Style::SetStrokePaint(const PaintRef& paint)
{
	if (fStrokePaint != paint) {
		if (fStrokePaint.Get() != NULL)
			fStrokePaint->RemoveListener(this);

		fStrokePaint = paint;

		if (fStrokePaint.Get() != NULL)
			fStrokePaint->AddListener(this);
		Notify();
	}
}

// SetStrokeProperties
void
Style::SetStrokeProperties(const ::StrokePropertiesRef& properties)
{
	if (fStrokeProperties != properties) {
		if (fStrokeProperties.Get() != NULL)
			fStrokeProperties->RemoveListener(this);

		fStrokeProperties = properties;

		if (fStrokeProperties.Get() != NULL)
			fStrokeProperties->AddListener(this);
		Notify();
	}
}

// ExtendBounds
void
Style::ExtendBounds(BRect& bounds) const
{
	if (fStrokePaint.Get() == NULL || fStrokePaint->Type() == Paint::NONE)
		return;

	if (fStrokeProperties == NULL)
		return;

	float maxStrokeWidth = fStrokeProperties->Width();
	if (fStrokeProperties->StrokePosition() == OutsideStroke)
		maxStrokeWidth *= 2.0f;

	if (fStrokeProperties->JoinMode() == MiterJoin
		|| fStrokeProperties->JoinMode() == MiterJoinRevert
		|| fStrokeProperties->JoinMode() == MiterJoinRound) {
		maxStrokeWidth *= fStrokeProperties->MiterLimit();
	}

	bounds.InsetBy(-maxStrokeWidth, -maxStrokeWidth);
}

// #pragma mark -

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

	if (paint != NULL) {
		paint->AddProperties(&paintProperties->Value(), flags);
		if (paint == fStrokePaint.Get() && paint->Type() != Paint::NONE
			&& fStrokeProperties.Get() != NULL) {
			fStrokeProperties->AddProperties(
				&paintProperties->Value(), flags);
		}
	} else {
		Paint::AddTypeProperty(&paintProperties->Value(), paintTypeID,
			Paint::NONE);
	}
}

/*static*/ Style&
Style::_NullStyle()
{
	static Style nullStyle;
	return nullStyle;
}
