/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "InspectorView.h"

// constructor
InspectorView::InspectorView()
	: PropertyListView()
{
}

// destructor
InspectorView::~InspectorView()
{
}

// #pragma mark -

// PropertyChanged
void
InspectorView::PropertyChanged(const Property* previous,
	const Property* current)
{
	PropertyListView::PropertyChanged(previous, current);
}

// PasteProperties
void
InspectorView::PasteProperties(const PropertyObject* object)
{
	PropertyListView::PasteProperties(object);
}

// IsEditingMultipleObjects
bool
InspectorView::IsEditingMultipleObjects()
{
	return PropertyListView::IsEditingMultipleObjects();
}

