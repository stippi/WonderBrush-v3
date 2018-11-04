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
	fStyle->SetFillPaint(PaintRef(new(std::nothrow) Paint(kBlack), true));
	fStyle->AddListener(this);
}

// constructor
Styleable::Styleable(const rgb_color& color)
	: BoundedObject()
	, fStyle(new(std::nothrow) ::Style(), true)
{
	fStyle->SetFillPaint(PaintRef(new(std::nothrow) Paint(color), true));
	fStyle->AddListener(this);
}

// constructor
Styleable::Styleable(const Styleable& other, CloneContext& context)
	: BoundedObject(other)
	, fStyle()
{
	context.Clone(other.fStyle.Get(), fStyle);
	fStyle->AddListener(this);
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
		// Forward notification
		NotifyAndUpdate();
	}
}

// #pragma mark -

// SetStyle
void
Styleable::SetStyle(::Style* style)
{
	if (style == fStyle.Get())
		return;

	if (fStyle.Get() != NULL)
		fStyle->RemoveListener(this);

	fStyle.SetTo(style);

	if (fStyle.Get() != NULL) {
		fStyle->AddListener(this);
		ObjectChanged(style);
	}
}


