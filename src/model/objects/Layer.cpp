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

// ObjectChanged
void
Layer::Listener::ObjectChanged(Layer* layer, Object* object, int32 index)
{
}

// AreaInvalidated
void
Layer::Listener::AreaInvalidated(Layer* layer, const BRect& area)
{
}

// ListenerAttached
void
Layer::Listener::ListenerAttached(Layer* layer)
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

// DefaultName
const char*
Layer::DefaultName() const
{
	return "Layer";
}

// HitTest
bool
Layer::HitTest(const BPoint& canvasPoint) const
{
	for (int32 i = CountObjects() - 1; i >= 0; i--) {
		Object* object = ObjectAtFast(i);
		if (object->HitTest(canvasPoint))
			return true;
	}
	return false;
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
	return reinterpret_cast<Object*>(fObjects.ItemAt(index));
}

// ObjectAtFast
Object*
Layer::ObjectAtFast(int32 index) const
{
	return reinterpret_cast<Object*>(fObjects.ItemAtFast(index));
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

// HasObject
bool
Layer::HasObject(Object* object) const
{
	return fObjects.HasItem(object);
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

	// notify listeners
	BList listeners(fListeners);
	count = listeners.CountItems();
	for (int32 i = 0; i < count; i++) {
		Listener* listener = (Listener*)listeners.ItemAtFast(i);
		listener->AreaInvalidated(this, visuallyChangedArea);
	}

	if (Parent())
		Parent()->Invalidate(visuallyChangedArea, Parent()->IndexOf(this));
}

// ObjectChanged
void
Layer::ObjectChanged(Object* object)
{
	int32 index = IndexOf(object);
	if (index < 0)
		return;
	// notify listeners
	BList listeners(fListeners);
	int32 count = listeners.CountItems();
	for (int32 i = 0; i < count; i++) {
		Listener* listener = (Listener*)listeners.ItemAtFast(i);
		listener->ObjectChanged(this, object, index);
	}
}

// HitTest
bool
Layer::HitTest(const BPoint& canvasPoint, Layer** _layer, Object** _object,
	bool recursive) const
{
	for (int32 i = CountObjects() - 1; i >= 0; i--) {
		Object* object = ObjectAtFast(i);
		Layer* subLayer = dynamic_cast<Layer*>(object);
		if (subLayer != NULL) {
			if (recursive
				&& subLayer->HitTest(canvasPoint, _layer, _object,
					recursive)) {
				return true;
			}
		} else if (object->HitTest(canvasPoint)) {
			if (_layer != NULL)
				*_layer = const_cast<Layer*>(this);
			if (_object != NULL)
				*_object = object;
			return true;
		}
	}
	return false;
}

// #pragma mark -

// AddListener
bool
Layer::AddListener(Listener* listener)
{
	if (!listener || fListeners.HasItem(listener))
		return false;
	if (!fListeners.AddItem(listener))
		return false;
	listener->ListenerAttached(this);
	return true;
}

// RemoveListener
void
Layer::RemoveListener(Listener* listener)
{
	fListeners.RemoveItem(listener);
}

// AddListenerRecursive
bool
Layer::AddListenerRecursive(Layer* layer, Listener* listener)
{
	// the document is locked and/or this is executed from within
	// a synchronous notification
	int32 count = layer->CountObjects();
	for (int32 i = 0; i < count; i++) {
		Object* object = layer->ObjectAtFast(i);
		Layer* subLayer = dynamic_cast<Layer*>(object);
		if (subLayer != NULL) {
			if (!AddListenerRecursive(subLayer, listener))
				return false;
		}
	}

	return layer->AddListener(listener);
}

// RemoveListenerRecursive
void
Layer::RemoveListenerRecursive(Layer* layer, Listener* listener)
{
	// the document is locked and/or this is executed from within
	// a synchronous notification
	int32 count = layer->CountObjects();
	for (int32 i = 0; i < count; i++) {
		Object* object = layer->ObjectAtFast(i);
		Layer* subLayer = dynamic_cast<Layer*>(object);
		if (subLayer != NULL)
			RemoveListenerRecursive(subLayer, listener);
	}

	layer->RemoveListener(listener);
}



