/*
 * Copyright 2007-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "Layer.h"

#include <new>

#include <stdio.h>

#include "BoundedObject.h"
#include "LayerSnapshot.h"
#include "OptionProperty.h"

using std::nothrow;

// constructor
Layer::Listener::Listener()
	:
	fUpdatesSuspended(0)
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

// SuspendUpdates
void
Layer::Listener::SuspendUpdates(bool suspend)
{
	if (!suspend && fUpdatesSuspended == 0)
		debugger("SuspendUpdates(false) with enabled updates.");

	if (suspend)
		fUpdatesSuspended++;
	else
		fUpdatesSuspended--;

	if (fUpdatesSuspended == 0)
		AllAreasInvalidated();
}

// UpdatesEnabled
bool
Layer::Listener::UpdatesEnabled() const
{
	return fUpdatesSuspended == 0;
}

// AreaInvalidated
void
Layer::Listener::AreaInvalidated(Layer* layer, const BRect& area)
{
}

// AllAreasInvalidated
void
Layer::Listener::AllAreasInvalidated()
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
	, fGlobalAlpha(255)
	, fBlendingMode(CompOpSrcOver)
	, fObjects(64)
	, fListeners(8)
{
}

// destructor
Layer::~Layer()
{
	for (int32 i = fObjects.CountItems() - 1; i >= 0; i--) {
		Object* object = (Object*)fObjects.ItemAtFast(i);
		object->RemoveReference();
	}
}

// #pragma mark -

// Unarchive
status_t
Layer::Unarchive(const BMessage* archive)
{
	status_t status = Object::Unarchive(archive);

	// TODO: ...

	return status;
}

// Archive
status_t
Layer::Archive(BMessage* into, bool deep) const
{
	status_t status = Object::Archive(into, deep);

	// TODO: ...

	return status;
}

// DefaultName
const char*
Layer::DefaultName() const
{
	return "Layer";
}

// AddProperties
void
Layer::AddProperties(PropertyObject* object, uint32 flags) const
{
	Object::AddProperties(object, flags);

	IntProperty* globalAlpha = new(std::nothrow) IntProperty(
		PROPERTY_OPACITY, fGlobalAlpha, 0, 255);
	if (globalAlpha == NULL || !object->AddProperty(globalAlpha)) {
		delete globalAlpha;
		return;
	}

	OptionProperty* blendingMode = new(std::nothrow) OptionProperty(
		PROPERTY_BLENDING_MODE);
	if (blendingMode == NULL || !object->AddProperty(blendingMode)) {
		delete blendingMode;
		return;
	}
	blendingMode->AddOption(CompOpClear, "Clear");
	blendingMode->AddOption(CompOpSrc, "Source");
	blendingMode->AddOption(CompOpDst, "Destination");
	blendingMode->AddOption(CompOpSrcOver, "Source over");
	blendingMode->AddOption(CompOpDstOver, "Destination over");
	blendingMode->AddOption(CompOpSrcIn, "Source in");
	blendingMode->AddOption(CompOpDstIn, "Destination in");
	blendingMode->AddOption(CompOpSrcOut, "Source out");
	blendingMode->AddOption(CompOpDstOut, "Destination out");
	blendingMode->AddOption(CompOpSrcAtop, "Source atop");
	blendingMode->AddOption(CompOpDstAtop, "Destination atop");
	blendingMode->AddOption(CompOpXor, "XOR");
	blendingMode->AddOption(CompOpPlus, "Plus");
	blendingMode->AddOption(CompOpMinus, "Minus");
	blendingMode->AddOption(CompOpMultiply, "Multiply");
	blendingMode->AddOption(CompOpScreen, "Screen");
	blendingMode->AddOption(CompOpOverlay, "Overlay");
	blendingMode->AddOption(CompOpDarken, "Darken");
	blendingMode->AddOption(CompOpLighten, "Lighten");
	blendingMode->AddOption(CompOpDodge, "Dodge");
	blendingMode->AddOption(CompOpColorBurn, "Color burn");
	blendingMode->AddOption(CompOpHardLight, "Hard light");
	blendingMode->AddOption(CompOpSoftLight, "Soft light");
	blendingMode->AddOption(CompOpDifference, "Difference");
	blendingMode->AddOption(CompOpExclusion, "Exclusion");
	blendingMode->AddOption(CompOpContrast, "Contrast");
	blendingMode->AddOption(CompOpInvert, "Invert");
	blendingMode->AddOption(CompOpInvertRGB, "Invert RGB");

	blendingMode->SetCurrentOptionID(fBlendingMode);
}

// SetToPropertyObject
bool
Layer::SetToPropertyObject(const PropertyObject* object, uint32 flags)
{
	AutoNotificationSuspender _(this);

	Object::SetToPropertyObject(object, flags);

	SetGlobalAlpha(object->Value(PROPERTY_OPACITY, (int32)fGlobalAlpha));

	OptionProperty* blendingMode = dynamic_cast<OptionProperty*>(
		object->FindProperty(PROPERTY_BLENDING_MODE));
	if (blendingMode != NULL) {
		SetBlendingMode(static_cast< ::BlendingMode>(
			blendingMode->CurrentOptionID()));
	}

	return HasPendingNotifications();
}

// TransformationChanged
void
Layer::TransformationChanged()
{
	NotifyListeners();
	// Override the Object version, which invalidates the whole parent,
	// to invalidate ourselves
	UpdateChangeCounter();
	Invalidate(Bounds(), 0);

	int32 count = CountObjects();
	for (int32 i = 0; i < count; i++) {
		Layer* layer = dynamic_cast<Layer*>(ObjectAtFast(i));
		if (layer != NULL)
			layer->TransformationChanged();
	}
}

// #pragma mark -

// Snapshot
ObjectSnapshot*
Layer::Snapshot() const
{
	return new (nothrow) LayerSnapshot(this);
}

// HitTest
bool
Layer::HitTest(const BPoint& canvasPoint)
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
//printf("%p->Layer::AddObject(%p, %ld)\n", this, object, index);
	if (object && fObjects.AddItem(object, index)) {
		object->AddReference();

		BList listeners(fListeners);
		int32 count = listeners.CountItems();
		for (int32 i = 0; i < count; i++) {
			Listener* listener = (Listener*)listeners.ItemAtFast(i);
			listener->ObjectAdded(this, object, index);
		}

		UpdateChangeCounter();
		object->SetParent(this);

		BoundedObject* boundedObject = dynamic_cast<BoundedObject*>(object);
		if (boundedObject != NULL)
			boundedObject->UpdateBounds();
		else
			Invalidate(Bounds(), index);

		return true;
	}
	return false;
}

// RemoveObject
Object*
Layer::RemoveObject(int32 index)
{
	Object* object = ObjectAt(index);
//printf("%p->Layer::RemoveObject(%ld): %p\n", this, index, object);
	if (object != NULL) {
		BRect invalidArea;
		BoundedObject* boundedObject = dynamic_cast<BoundedObject*>(object);
		if (boundedObject != NULL)
			invalidArea = boundedObject->TransformedBounds();
		else
			invalidArea = Bounds();

		object->SetParent(NULL);
		UpdateChangeCounter();

		if (fObjects.RemoveItem(index) == NULL) {
			// Should not happen, but roll back in any case.
			object->SetParent(this);
			return NULL;
		}

		BList listeners(fListeners);
		int32 count = listeners.CountItems();
		for (int32 i = 0; i < count; i++) {
			Listener* listener = (Listener*)listeners.ItemAtFast(i);
			listener->ObjectRemoved(this, object, index);
		}

		Invalidate(invalidArea, index);
		object->RemoveReference();
	}
	return object;
}

// RemoveObject
bool
Layer::RemoveObject(Object* object)
{
	return RemoveObject(IndexOf(object)) != NULL;
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

// SuspendUpdates
void
Layer::SuspendUpdates(bool suspend)
{
	BList listeners(fListeners);
	int32 count = listeners.CountItems();
	for (int32 i = 0; i < count; i++) {
		Listener* listener = (Listener*)listeners.ItemAtFast(i);
		listener->SuspendUpdates(suspend);
	}
}

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
		listener->SuspendUpdates(true);
		listener->AreaInvalidated(this, visuallyChangedArea);
	}

	if (Parent())
		Parent()->Invalidate(visuallyChangedArea, Parent()->IndexOf(this));

	for (int32 i = 0; i < count; i++) {
		Listener* listener = (Listener*)listeners.ItemAtFast(i);
		listener->SuspendUpdates(false);
	}
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

// SetGlobalAlpha
void
Layer::SetGlobalAlpha(uint8 globalAlpha)
{
	if (fGlobalAlpha == globalAlpha)
		return;

	fGlobalAlpha = globalAlpha;
	UpdateChangeCounter();
	InvalidateParent(fBounds);
	Notify();
}

// SetBlendingMode
void
Layer::SetBlendingMode(::BlendingMode blendingMode)
{
	if (blendingMode < kMinBlendingMode)
		blendingMode = kMinBlendingMode;
	if (blendingMode > kMaxBlendingMode)
		blendingMode = kMaxBlendingMode;

	if (fBlendingMode == blendingMode)
		return;

	fBlendingMode = blendingMode;
	UpdateChangeCounter();
	InvalidateParent(fBounds);
	Notify();
}

