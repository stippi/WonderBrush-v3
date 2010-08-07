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
	SetStyle(NULL);
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

	fStyle->SetToPropertyObject(object, flags);

	return HasPendingNotifications();
}

// #pragma mark -

void
Styleable::ObjectChanged(const Notifier* object)
{
	if (object == fStyle.Get()) {
		UpdateChangeCounter();
		UpdateBounds();
		// Forward notification
		Notify();
	}
}

// #pragma mark -

// SetStyle
void
Styleable::SetStyle(::Style* style)
{
	if (style == fStyle.Get())
		return;

	if (fStyle.Get())
		fStyle->RemoveListener(this);

	if (fStyle.SetTo(style)) {
		fStyle->AddListener(this);
		ObjectChanged(style);
	}
}


