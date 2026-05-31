#include "BoundedObject.h"
#include "Layer.h"
#include "LayerSnapshot.h"

BoundedObject::BoundedObject()
	: Object()
	, fOpacity(255)
	, fTransformedBounds()
{
}

BoundedObject::BoundedObject(const BoundedObject& other)
	: Object(other)
	, fOpacity(other.fOpacity)
	, fTransformedBounds(other.fTransformedBounds)
{
}

BoundedObject::~BoundedObject() {}
void BoundedObject::AddProperties(PropertyObject*, uint32) const {}
bool BoundedObject::SetToPropertyObject(const PropertyObject*, uint32) { return false; }
void BoundedObject::TransformationChanged() {}
void BoundedObject::InitBounds() {}
void BoundedObject::UpdateBounds() {}
void BoundedObject::SetOpacity(uint8 opacity) { fOpacity = opacity; }
void BoundedObject::NotifyAndUpdate() {}

ObjectSnapshot::ObjectSnapshot(const Object*)
	: Transformable()
	, fChangeCounter(0)
	, fLayoutedState()
	, fIsVisible(true)
{
}

ObjectSnapshot::~ObjectSnapshot() {}
bool ObjectSnapshot::Sync() { return false; }
void ObjectSnapshot::Layout(LayoutContext&, uint32) {}
void ObjectSnapshot::PrepareRendering(BRect) {}
void ObjectSnapshot::Render(RenderEngine&, RenderBuffer*, BRect) const {}
void ObjectSnapshot::RebuildAreaForDirtyArea(BRect&) const {}

LayerSnapshot::LayerSnapshot(const ::Layer* layer)
	: ObjectSnapshot(layer)
	, fOriginal(layer)
	, fObjects(8)
	, fBounds()
	, fBitmap(0)
	, fGlobalAlpha(255)
	, fBlendingMode(CompOpSrcOver)
{
}

LayerSnapshot::~LayerSnapshot() {}
const Object* LayerSnapshot::Original() const { return fOriginal; }
bool LayerSnapshot::Sync() { return false; }
void LayerSnapshot::Layout(LayoutContext&, uint32) {}
void LayerSnapshot::Render(RenderEngine&, RenderBuffer*, BRect) const {}
BRect LayerSnapshot::Bounds() const { return fBounds; }
BRect LayerSnapshot::Render(RenderEngine&, BRect area, RenderBuffer*, RenderBuffer*,
	BRegion&, int32&) const { return area; }
ObjectSnapshot* LayerSnapshot::ObjectAt(int32) const { return 0; }
ObjectSnapshot* LayerSnapshot::ObjectAtFast(int32) const { return 0; }
int32 LayerSnapshot::CountObjects() const { return 0; }
void LayerSnapshot::_Sync() {}
void LayerSnapshot::_MakeEmpty() {}
