/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#include "Layer.h"

#include <new>

#include "LayerSnapshot.h"


using std::nothrow;

// constructor
Layer::Listener::Listener()
{
}

// destructor
Layer::Listener::~Listener()
{
}

// ObjectAdded
void
Layer::Listener::ObjectAdded(Layer* layer, Object* object, int32 index)
{
}

// ObjectRemoved
void
Layer::Listener::ObjectRemoved(Layer* layer, Object* object, int32 index)
{
}

// AreaInvalidated
void
Layer::Listener::AreaInvalidated(Layer* layer, const BRect& area)
{
}

// #pragma mark -

// constructor
Layer::Layer(const BRect& bounds)
	: fBounds(bounds)
	, fObjects(64)
	, fListeners(8)
{
}

// destructor
Layer::~Layer()
{
}

// #pragma mark -

// Snapshot
ObjectSnapshot*
Layer::Snapshot() const
{
	return new (nothrow) LayerSnapshot(this);
}

// #pragma mark -

// AddObject
bool
Layer::AddObject(Object* object)
{
	return AddObject(object, CountObjects());
}

// AddObject
bool
Layer::AddObject(Object* object, int32 index)
{
	if (object && fObjects.AddItem(object, index)) {
		BList listeners(fListeners);
		int32 count = listeners.CountItems();
		for (int32 i = 0; i < count; i++) {
			Listener* listener = (Listener*)listeners.ItemAtFast(i);
			listener->ObjectAdded(this, object, index);
		}

		object->SetParent(this);
		UpdateChangeCounter();
		return true;
	}
	return false;
}

// RemoveObject
Object*
Layer::RemoveObject(int32 index)
{
	Object* object = (Object*)fObjects.RemoveItem(index);
	if (object) {
		BList listeners(fListeners);
		int32 count = listeners.CountItems();
		for (int32 i = 0; i < count; i++) {
			Listener* listener = (Listener*)listeners.ItemAtFast(i);
			listener->ObjectRemoved(this, object, index);
		}

		object->SetParent(NULL);
		UpdateChangeCounter();
	}
	return object;
}

// ObjectAt
Object*
Layer::ObjectAt(int32 index) const
{
	return (Object*)fObjects.ItemAt(index);
}

// ObjectAtFast
Object*
Layer::ObjectAtFast(int32 index) const
{
	return (Object*)fObjects.ItemAtFast(index);
}

// IndexOf
int32
Layer::IndexOf(Object* object) const
{
	return fObjects.IndexOf(object);
}

// CountObjects
int32
Layer::CountObjects() const
{
	return fObjects.CountItems();
}

// #pragma mark -

// Invalidate
void
Layer::Invalidate(const BRect& area, int32 objectIndex)
{
	// calculate the *visually changed area* from the lowest
	// changed object to the top object, giving each object
	// a chance to extend the area
	BRect visuallyChangedArea = area;
	int32 count = CountObjects();
	for (int32 i = objectIndex; i < count; i++) {
		Object* object = ObjectAtFast(i);
		object->ExtendDirtyArea(visuallyChangedArea);
	}

	if (Parent())
		Parent()->Invalidate(visuallyChangedArea, Parent()->IndexOf(this));

	// notify listeners
	BList listeners(fListeners);
	count = listeners.CountItems();
	for (int32 i = 0; i < count; i++) {
		Listener* listener = (Listener*)listeners.ItemAtFast(i);
		listener->AreaInvalidated(this, visuallyChangedArea);
	}
}

// AddListener
bool
Layer::AddListener(Listener* listener)
{
	if (!listener || fListeners.HasItem(listener))
		return false;
	return fListeners.AddItem(listener);
}

// RemoveListener
void
Layer::RemoveListener(Listener* listener)
{
	fListeners.RemoveItem(listener);
}
