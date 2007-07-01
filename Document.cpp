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
	: RWLocker("document rw lock")
	, fCommandStack(new (nothrow) ::CommandStack(this))
	, fRootLayer(new (nothrow) Layer(bounds))
	, fListeners(8)
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
	if (!listener || fListeners.HasItem(listener))
		return false;
	return fListeners.AddItem(listener);
}

// RemoveListener
void
Document::RemoveListener(Listener* listener)
{
	fListeners.RemoveItem(listener);
}
