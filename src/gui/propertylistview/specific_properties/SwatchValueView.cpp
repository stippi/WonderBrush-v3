/*
 * Copyright 2006-2013, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "SwatchValueView.h"

#include <stdio.h>

// constructor
SwatchValueView::SwatchValueView(const char* name, BMessage* message,
	BHandler* target, rgb_color color, float width, float height)
	: SwatchView(name, message, target, color, width, height, B_NO_BORDER)
{
	uint32 flags = Flags();
	flags |= B_NAVIGABLE;
	SetFlags(flags);
}

// destructor
SwatchValueView::~SwatchValueView()
{
}

// MakeFocus
void
SwatchValueView::MakeFocus(bool focused)
{
	BView::MakeFocus(focused);
	if (BView* parent = Parent())
		parent->Invalidate();
}

// PlatformDraw
void
SwatchValueView::PlatformDraw(PlatformDrawContext& drawContext)
{
	BRect b(Bounds());
	if (BView* parent = Parent()) {
		SetLowColor(tint_color(parent->LowColor(), B_DARKEN_1_TINT));
		StrokeRect(b, B_SOLID_LOW);
		b.InsetBy(1.0, 1.0);
	}
	DrawSwatch(b, drawContext);
}


// MouseDown
void
SwatchValueView::MouseDown(BPoint where)
{
	// forward click
	if (BView* parent = Parent())
		parent->MouseDown(ConvertToParent(where));

	SwatchView::MouseDown(where);
}
