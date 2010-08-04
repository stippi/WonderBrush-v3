/*
 * Copyright 2009-2010, Stephan Aßmus <superstippi@gmx.de>. All rights reserved.
 */

#include "Styleable.h"

#include <new>

#include <stdio.h>

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
}

// #pragma mark -

// AddProperties
void
Styleable::AddProperties(PropertyObject* object, uint32 flags) const
{
	BoundedObject::AddProperties(object, flags);
	fStyle->AddProperties(object, flags);
}

// SetToPropertyObject
bool
Styleable::SetToPropertyObject(const PropertyObject* object, uint32 flags)
{
	AutoNotificationSuspender _(this);

	BoundedObject::SetToPropertyObject(object, flags);

	AutoNotificationSuspender styleSuspender(fStyle.Get());
	if (fStyle->SetToPropertyObject(object, flags))
		Notify();

	if (HasPendingNotifications()) {
		UpdateChangeCounter();
		UpdateBounds();
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


