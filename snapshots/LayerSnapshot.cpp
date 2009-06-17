/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#include "LayerSnapshot.h"

#include <new>
#include <string.h>

#include <Bitmap.h>
#include <Region.h>

#include "bitmap_support.h"

#include "Layer.h"
#include "LayoutContext.h"
#include "Object.h"


using std::nothrow;


// constructor
LayerSnapshot::LayerSnapshot(const ::Layer* layer)
	: ObjectSnapshot(layer)
	, fOriginal(layer)
	, fObjects(20)
	, fBitmap(new (nothrow) BBitmap(layer->Bounds(), B_BITMAP_NO_SERVER_LINK,
		B_RGBA32))
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
	LayoutState state(context.State());
	context.PushState(&state);

	// TODO: Keep Transformable as a member, don't inherit it.
	context.SetTransformation(*this);

	int32 count = CountObjects();
	for (int32 i = 0; i < count; i++) {
		ObjectSnapshot* snapshot = ObjectAtFast(i);
		snapshot->Layout(context, flags);
	}

	context.PopState();
}

// Render
void
LayerSnapshot::Render(RenderEngine& engine, BBitmap* bitmap, BRect area) const
{
	blend_area(fBitmap, bitmap, area);
}

// #pragma mark -

// Bounds
BRect
LayerSnapshot::Bounds() const
{
	if (fBitmap)
		return fBitmap->Bounds();
	return BRect(0, 0, -1, -1);
}

// Render
BRect
LayerSnapshot::Render(RenderEngine& engine, BRect area, BBitmap* bitmap,
	BBitmap* cacheBitmap, BRegion& validCacheRegion, int32& cacheLevel) const
{
	area = area & bitmap->Bounds();
	if (!area.IsValid())
		return area;

	// first pass, give every object snapshot a chance to
	// extend the visually changed area, start with the lowest
	// changed object
	int32 count = CountObjects();
//	lowestChangedObject = max_c(0, min_c(count - 1, lowestChangedObject));

	BRect visuallyChangedArea = area;

	// calculate the required *rebuild area* at each object
	// index, from the top object to the lowest object
	BRect dirtyAreas[count];
	BRect rebuildArea = visuallyChangedArea;
	for (int32 i = count - 1; i >= 0; i--) {
		dirtyAreas[i] = rebuildArea;
		ObjectSnapshot* object = ObjectAtFast(i);
		object->RebuildAreaForDirtyArea(rebuildArea);
	}

	// begin rendering

	rebuildArea = rebuildArea & bitmap->Bounds();
	int32 firstObject = 0;

//	// Figure out until which object level we can reuse the cache bitmap,
//	// if at all.
//	BRegion cacheTest(rebuildArea);
//	cacheTest.Exclude(&validCacheRegion);
//	bool canUseCache = false;
//	if (cacheLevel < lowestChangedObject && cacheTest.CountRects() == 0) {
////printf("can use cache\n");
//		// we can use the cache
//		firstObject = cacheLevel + 1;
//		canUseCache = true;
//	} else {
//		// we need to build the cache
//		if (cacheLevel != lowestChangedObject - 1) {
//			cacheLevel = lowestChangedObject - 1;
//			validCacheRegion.MakeEmpty();
////printf("start building cache\n");
//		} else {
////printf("continue building cache\n");
//		}
//	}

	// clear the bitmap, or transfer the area from the cache
//printf("rebuild: "); rebuildArea.PrintToStream();

//	if (cacheBitmap && canUseCache) {
//		// use the cache
//		copy_area(cacheBitmap, bitmap, rebuildArea);
//	} else {
		// start clean
		uint8* bits = (uint8*)bitmap->Bits();
		uint32 bytes = (rebuildArea.IntegerWidth() + 1) * 4;
		uint32 height = rebuildArea.IntegerHeight() + 1;
		uint32 bpr = bitmap->BytesPerRow();
	
		bits += (int32)rebuildArea.top * bpr;
		bits += (int32)rebuildArea.left * 4;
	
		// clean out bitmap
		for (uint32 y = 0; y < height; y++) {
			memset(bits, 0, bytes);
			bits += bpr;
		}
//	}

	// render objects
	BRect layerBounds = bitmap->Bounds();

	for (int32 i = firstObject; i < count; i++) {
		ObjectSnapshot* object = ObjectAtFast(i);
		object->PrepareRendering(layerBounds);
		object->Render(engine, bitmap, dirtyAreas[i]);
//		if (cacheBitmap && i == cacheLevel) {
//			copy_area(bitmap, cacheBitmap, dirtyAreas[i]);
//			validCacheRegion.Include(dirtyAreas[i]);
//		}
	}

	// return the final visually changed area
	visuallyChangedArea = visuallyChangedArea & bitmap->Bounds();
//printf("transfer: "); largestDirtyArea.PrintToStream();
	copy_area(bitmap, fBitmap, visuallyChangedArea);
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
	int32 count = fOriginal->CountObjects();
	for (int32 i = 0; i < count; i++) {
		Object* object = fOriginal->ObjectAtFast(i);
		ObjectSnapshot* snapshot = ObjectAt(i);

		if (!snapshot) {
			// create new snapshot of object at index
			fObjects.AddItem(object->Snapshot());
			continue;
		}
		if (snapshot->Original() == object) {
			// correct snapshot already at index
			snapshot->Sync();
			continue;
		}
		
		while (snapshot && snapshot->Original() != object) {
			// delete all snapshots until they match again
			fObjects.RemoveItem(i);
			delete snapshot;
			snapshot = ObjectAt(i);
		}
	}

	if (!fBitmap || fOriginal->Bounds() != fBitmap->Bounds()) {
		delete fBitmap;
		fBitmap = new (nothrow) BBitmap(fOriginal->Bounds(),
			B_BITMAP_NO_SERVER_LINK, B_RGBA32);
	}
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

