/*
 * Copyright 2007-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#include "LayerSnapshot.h"

#include <new>

#include <stdio.h>
#include <string.h>

#include <Region.h>

#include "Layer.h"
#include "LayoutContext.h"
#include "Object.h"
#include "RenderBuffer.h"
#include "RenderEngine.h"

using std::nothrow;

// constructor
LayerSnapshot::LayerSnapshot(const ::Layer* layer)
	: ObjectSnapshot(layer)
	, fOriginal(layer)
	, fObjects(20)
	, fBounds()
	, fBitmap(NULL)
	, fGlobalAlpha(255)
	, fBlendingMode(CompOpSrcOver)
{
	_Sync();
}

// destructor
LayerSnapshot::~LayerSnapshot()
{
	_MakeEmpty();
	delete fBitmap;
}

// #pragma mark -

// Original
const Object*
LayerSnapshot::Original() const
{
	return fOriginal;
}

// Sync
bool
LayerSnapshot::Sync()
{
	if (!ObjectSnapshot::Sync())
		return false;

	_Sync();

	return true;
}

// #pragma mark -

// Layout
void
LayerSnapshot::Layout(LayoutContext& context, uint32 flags)
{
//printf("%p->LayerSnapshot::Layout()\n", Original());
	// Allocate or resize bitmap for caching layer contents
	BRect zoomedBounds(fBounds);
	zoomedBounds.left = floorf(zoomedBounds.left * context.ZoomLevel());
	zoomedBounds.top = floorf(zoomedBounds.top * context.ZoomLevel());
	zoomedBounds.right = ceilf(zoomedBounds.right * context.ZoomLevel());
	zoomedBounds.bottom = ceilf(zoomedBounds.bottom * context.ZoomLevel());
	if (fBitmap == NULL || zoomedBounds != fBitmap->Bounds()) {
//printf("  resizing bitmap\n");
		delete fBitmap;
		fBitmap = new (nothrow) RenderBuffer(zoomedBounds);
		if (!fBitmap->IsValid())
			return;
		fBitmap->Clear(zoomedBounds, (rgb_color){ 0, 0, 0, 0 });
	}

	ObjectSnapshot::Layout(context, flags);

	int32 count = CountObjects();
	for (int32 i = 0; i < count; i++) {
		ObjectSnapshot* snapshot = ObjectAtFast(i);

		LayoutState objectState(context.State());
		context.PushState(&objectState);

		snapshot->Layout(context, flags);

		context.PopState();
	}
}

// Render
void
LayerSnapshot::Render(RenderEngine& engine, RenderBuffer* bitmap,
	BRect area) const
{
//printf("%p->LayerSnapshot::Render(BRect(%.1f, %.1f, %.1f, %.1f))\n", fOriginal,
//area.left, area.top, area.right, area.bottom);
	area = area & bitmap->Bounds();
	if (fBitmap == NULL)
		debugger("Layer bitmap not allocated!");
	if (fBitmap->Bounds() != bitmap->Bounds())
		debugger("Layer bitmap has wrong size!");
	engine.BlendArea(fBitmap, area, fGlobalAlpha, fBlendingMode);
}

// #pragma mark -

// Bounds
BRect
LayerSnapshot::Bounds() const
{
	return fBounds;
}

// Render
BRect
LayerSnapshot::Render(RenderEngine& engine, BRect area, RenderBuffer* bitmap,
	RenderBuffer* cacheBitmap, BRegion& validCacheRegion,
	int32& cacheLevel) const
{
//printf("%p->LayerSnapshot::Render(BRect(%.1f, %.1f, %.1f, %.1f)) objects\n",
//fOriginal, area.left, area.top, area.right, area.bottom);
	area = area & bitmap->Bounds();
	if (!area.IsValid())
		return area;

	// Check bitmap size matches (for debugging purposes)
	if (fBitmap == NULL || bitmap->Bounds() != fBitmap->Bounds()) {
		printf("Layer bitmap has wrong size or is not allocated!");
		bitmap->Bounds().PrintToStream();
		fBitmap->Bounds().PrintToStream();
	}

	// first pass, give every object snapshot a chance to
	// extend the visually changed area, start with the lowest
	// changed object
	int32 count = CountObjects();

	BRect visuallyChangedArea = area;

	// calculate the required *rebuild area* at each object
	// index, from the top object to the lowest object
	BRect dirtyAreas[count];
	BRect rebuildArea = visuallyChangedArea;
	for (int32 i = count - 1; i >= 0; i--) {
		dirtyAreas[i] = rebuildArea;
		ObjectSnapshot* object = ObjectAtFast(i);
		if (object->IsVisible())
			object->RebuildAreaForDirtyArea(rebuildArea);
	}

	// begin rendering

	rebuildArea = rebuildArea & bitmap->Bounds();

	// start clean
	uint8* bits = (uint8*)bitmap->Bits();
	uint32 bytes = (rebuildArea.IntegerWidth() + 1) * 8;
	uint32 height = rebuildArea.IntegerHeight() + 1;
	uint32 bpr = bitmap->BytesPerRow();

	bits += (int32)rebuildArea.top * bpr;
	bits += (int32)rebuildArea.left * 8;

	// clean out bitmap
	for (uint32 y = 0; y < height; y++) {
		memset(bits, 0, bytes);
		bits += bpr;
	}

	// render objects
	BRect layerBounds = bitmap->Bounds();

	engine.AttachTo(bitmap);

	for (int32 i = 0; i < count; i++) {
		ObjectSnapshot* object = ObjectAtFast(i);
		if (!object->IsVisible())
			continue;
		object->PrepareRendering(layerBounds);

		engine.SetClipping(dirtyAreas[i]);

		object->Render(engine, bitmap, dirtyAreas[i]);
	}

	// return the final visually changed area
	visuallyChangedArea = visuallyChangedArea & bitmap->Bounds();
//printf("transfer: "); largestDirtyArea.PrintToStream();
	bitmap->CopyTo(fBitmap, visuallyChangedArea);
	return visuallyChangedArea;
}

// #pragma mark -

// ObjectAt
ObjectSnapshot*
LayerSnapshot::ObjectAt(int32 index) const
{
	return reinterpret_cast<ObjectSnapshot*>(fObjects.ItemAt(index));
}

// ObjectAtFast
ObjectSnapshot*
LayerSnapshot::ObjectAtFast(int32 index) const
{
	return reinterpret_cast<ObjectSnapshot*>(fObjects.ItemAtFast(index));
}

// CountObjects
int32
LayerSnapshot::CountObjects() const
{
	return fObjects.CountItems();
}

// #pragma mark -

// _Sync
void
LayerSnapshot::_Sync()
{
//printf("%p->LayerSnapshot::_Sync()\n", Original());
	BList removedSnapshots;

	int32 count = fOriginal->CountObjects();
	int32 i = 0;
	for (; i < count; i++) {
		Object* object = fOriginal->ObjectAtFast(i);
		ObjectSnapshot* snapshot = ObjectAt(i);

		while (snapshot != NULL) {
			if (snapshot->Original() == object) {
				// correct snapshot already at index
//printf("%p - [%ld] syncing %p\n", Original(), i, snapshot);
				snapshot->Sync();
				break;
			}
			// delete all snapshots until they match again
//printf("%p - [%ld] removing %p\n", Original(), i, snapshot);
			fObjects.RemoveItem(i);
			removedSnapshots.AddItem(snapshot);
			snapshot = ObjectAt(i);
		}

		if (snapshot == NULL) {
			// create new snapshot of object at index
			bool foundRemoved = false;
			int32 removedCount = removedSnapshots.CountItems();
			for (int32 j = 0; j < removedCount; j++) {
				ObjectSnapshot* removedSnapshot
					= reinterpret_cast<ObjectSnapshot*>(
						removedSnapshots.ItemAtFast(j));
				if (removedSnapshot->Original() == object) {
					removedSnapshots.RemoveItem(j);
					fObjects.AddItem(removedSnapshot);
//printf("%p - [%ld] syncing %p (removed)\n", Original(), i, removedSnapshot);
					removedSnapshot->Sync();
					foundRemoved = true;
					break;
				}
			}
			if (!foundRemoved) {
				snapshot = object->Snapshot();
//printf("%p - [%ld] cloning %p\n", Original(), i, snapshot);
				fObjects.AddItem(snapshot);
			}
		}
	}

	// In case all object snapshots matched, we still need to remove
	// any additional snapshots at the end.
	count = CountObjects();
	for (; i < count; i++) {
		ObjectSnapshot* snapshot = reinterpret_cast<ObjectSnapshot*>(
			fObjects.RemoveItem(i));
//printf("%p - deleting [%ld] %p\n", Original(), i, snapshot);
		delete snapshot;
	}

	// Delete the old snapshots we no longer needed
	for (int32 i = removedSnapshots.CountItems() - 1; i >= 0; i--) {
		ObjectSnapshot* snapshot = reinterpret_cast<ObjectSnapshot*>(
			removedSnapshots.ItemAtFast(i));
//printf("%p - deleting removed %p\n", Original(), snapshot);
		delete snapshot;
	}

	fBounds = fOriginal->Bounds();
	fGlobalAlpha = fOriginal->GlobalAlpha();
	fBlendingMode = fOriginal->BlendingMode();
//printf("%p->LayerSnapshot::_Sync() - done\n", Original());
}

// _MakeEmpty
void
LayerSnapshot::_MakeEmpty()
{
	int32 count = CountObjects();
	for (int32 i = 0; i < count; i++)
		delete ObjectAtFast(i);
	fObjects.MakeEmpty();
}

