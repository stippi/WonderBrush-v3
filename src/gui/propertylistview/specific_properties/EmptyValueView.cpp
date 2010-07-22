/*
 * Copyright 2006-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "EmptyValueView.h"

#include <stdio.h>

#include "ui_defines.h"

// constructor
EmptyValueView::EmptyValueView(Property* property)
	: PropertyEditorView(),
	  fProperty(property)
{
}

// destructor
EmptyValueView::~EmptyValueView()
{
}

// Draw
void
EmptyValueView::Draw(BRect updateRect)
{
	// background
	FillRect(Bounds(), B_SOLID_LOW);
}

// MouseDown
void
EmptyValueView::MouseDown(BPoint where)
{
	MakeFocus(true);	
}

// SetEnabled
void
EmptyValueView::SetEnabled(bool enabled)
{
}

// AdoptProperty
bool
EmptyValueView::AdoptProperty(Property* property)
{
	fProperty = property;
	return true;
}

// GetProperty
Property*
EmptyValueView::GetProperty() const
{
	return fProperty;
}

