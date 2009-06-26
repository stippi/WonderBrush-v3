/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#include "Document.h"

#include <new>

#include "CommandStack.h"
#include "Layer.h"
#include "Object.h"
#include "Path.h"
#include "Style.h"


using std::nothrow;

// constructor
Document::Listener::Listener()
{
}

// destructor
Document::Listener::~Listener()
{
}

// #pragma mark -

// constructor
Document::Document(const BRect& bounds)
	:
	RWLocker("document rw lock"),
	fCommandStack(new (nothrow) ::CommandStack(this)),
	fRootLayer(new (nothrow) Layer(bounds)),
	fGlobalPaths(),
	fGlobalStyles(),
	fListeners(8)
{
}

// destructor
Document::~Document()
{
	delete fCommandStack;
	delete fRootLayer;
}

// #pragma mark -

// InitCheck
status_t
Document::InitCheck() const
{
	return fCommandStack && fRootLayer ? B_OK : B_NO_MEMORY;
}

// Bounds
BRect
Document::Bounds() const
{
	return fRootLayer->Bounds();
}

// #pragma mark -

// AddListener
bool
Document::AddListener(Listener* listener)
{
	if (listener == NULL || fListeners.HasItem(listener))
		return false;
	return fListeners.AddItem(listener);
}

// RemoveListener
void
Document::RemoveListener(Listener* listener)
{
	fListeners.RemoveItem(listener);
}

// #pragma mark -

// HasLayer
bool
Document::HasLayer(Layer* layer) const
{
	if (!layer)
		return false;
	return _HasLayer(fRootLayer, layer);
}

// _HasLayer
bool
Document::_HasLayer(Layer* parent, Layer* child) const
{
	if (parent == child)
		return true;

	int32 count = parent->CountObjects();
	for (int32 i = 0; i < count; i++) {
		Object* object = parent->ObjectAtFast(i);
		Layer* subLayer = dynamic_cast<Layer*>(object);
		if (subLayer && _HasLayer(subLayer, child))
			return true;
	}

	return false;
}

