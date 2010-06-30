/*
 * Copyright 2009-2010, Stephan Aßmus <superstippi@gmx.de>. All rights reserved.
 */

#include "Styleable.h"

#include <new>

#include "Paint.h"
#include "Style.h"
#include "ui_defines.h"

// constructor
Styleable::Styleable()
	: BoundedObject()
	, fStyle(new(std::nothrow) ::Style(), true)
{
	fStyle->SetFillPaint(Paint(kBlack));
}

// constructor
Styleable::Styleable(const rgb_color& color)
	: BoundedObject()
	, fStyle(new(std::nothrow) ::Style(), true)
{
	fStyle->SetFillPaint(Paint(color));
}

// destructor
Styleable::~Styleable()
{
	fStyle->RemoveReference();
}

// #pragma mark -

// AddProperties
void
Styleable::AddProperties(PropertyObject* object) const
{
	BoundedObject::AddProperties(object);
	fStyle->AddProperties(object);
}

// SetToPropertyObject
bool
Styleable::SetToPropertyObject(const PropertyObject* object)
{
	AutoNotificationSuspender _(this);

	BoundedObject::SetToPropertyObject(object);

	AutoNotificationSuspender styleSuspender(fStyle.Get());
	fStyle->SetToPropertyObject(object);

	if (HasPendingNotifications() || fStyle->HasPendingNotifications()) {
		UpdateChangeCounter();
		InvalidateParent(TransformedBounds());
		return true;
	}
	return false;
}

// #pragma mark -

// SetStyle
void
Styleable::SetStyle(::Style* style)
{
	if (fStyle.SetTo(style)) {
		UpdateChangeCounter();
		InvalidateParent(TransformedBounds());
	}
}


