/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#define USE_CACHING 1
#define DEBUG_CACHING 0


#include "RenderThread.h"

#include <new>
#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <Message.h>

#if DEBUG_CACHING
#  include <String.h>
#  include <View.h>
#  include <Window.h>
#endif

#include "bitmap_support.h"

#include "Layer.h"
#include "LayerSnapshot.h"
#include "ObjectSnapshot.h"
#include "RenderManager.h"


using std::nothrow;

enum {
	MSG_RENDER = 'rndr'
};


#if DEBUG_CACHING
class CacheView : public BView {
public:
		CacheView(BRect frame, BBitmap* cacheBitmap)
			: BView(frame, "", B_FOLLOW_ALL, B_WILL_DRAW)
			, fCacheBitmap(cacheBitmap)
		{
			SetViewColor(B_TRANSPARENT_COLOR);
		}
		virtual ~CacheView()
		{
		}
		virtual void Draw(BRect updateRect)
		{
			BRegion invalidCache(updateRect);
			invalidCache.Exclude(&fValidCache);
			SetHighColor(255, 0, 0);
			FillRegion(&invalidCache);

			ConstrainClippingRegion(&fValidCache);
			DrawBitmap(fCacheBitmap, B_ORIGIN);
		}

		void SetValidCache(const BRegion& region)
		{
			if (!LockLooper())
				return;

			fValidCache = region;

			PushState();
			Draw(Bounds());
			PopState();

			UnlockLooper();
		}
private:
		BRegion fValidCache;
		BBitmap* fCacheBitmap;
};

class BitmapView : public BView {
public:
		BitmapView(BRect frame, BBitmap* bitmap)
			: BView(frame, "", B_FOLLOW_ALL, B_WILL_DRAW)
			, fBitmap(bitmap)
		{
			SetViewColor(B_TRANSPARENT_COLOR);
		}
		virtual ~BitmapView()
		{
		}
		virtual void Draw(BRect updateRect)
		{
			DrawBitmap(fBitmap, B_ORIGIN);
		}

		void Redraw()
		{
			if (!LockLooper())
				return;

			PushState();
			Draw(Bounds());
			PopState();

			UnlockLooper();
		}
private:
		BBitmap* fBitmap;
};
#endif

// #pragma mark -

class RenderThread::LayerBitmap {
 public:
								LayerBitmap(LayerSnapshot* layer);
	virtual						~LayerBitmap();

	inline	const LayerSnapshot* Layer() const
									{ return fLayer; }

			status_t			InitCheck() const;
			BRect				Render(BRect area, int32 lowestChangedObject);

			const BBitmap*		Bitmap() const
									{ return &fLayerBitmap; }

 private:
			LayerSnapshot*		fLayer;

			BBitmap				fLayerBitmap;
#if USE_CACHING
			BBitmap				fCacheBitmap;
			int32				fCacheLevel;
			BRegion				fValidCache;

#  if DEBUG_CACHING
			BWindow*			fCacheWindow;
			CacheView*			fCacheView;
			BWindow*			fBitmapWindow;
			BitmapView*			fBitmapView;
#  endif
#endif
};

// constructor
RenderThread::LayerBitmap::LayerBitmap(LayerSnapshot* layer)
	: fLayer(layer)
	, fLayerBitmap(layer->Bounds(), 0, B_RGBA32)
#if USE_CACHING
	, fCacheBitmap(layer->Bounds(), 0, B_RGBA32)
	, fCacheLevel(-1)
	, fValidCache()
#endif
{
#  if DEBUG_CACHING
	uint32 windowFlags = B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_NOT_CLOSABLE;
	BString helper;
	helper = "Bitmap Region ";
	helper << index + 1;
	BRect frame = manager->Bounds().OffsetByCopy(90, 115);
	frame.OffsetBy(frame.Width() + 30, index * (frame.Height() + 40));
	fBitmapWindow = new BWindow(frame, helper.String(), B_TITLED_WINDOW, windowFlags);
	fBitmapView = new BitmapView(fBitmapWindow->Bounds(), fBitmap);
	fBitmapWindow->AddChild(fBitmapView);
	fBitmapWindow->Show();

	helper = "Invalid Cache Region ";
	helper << index + 1;
	frame.OffsetBy(frame.Width() + 30, 0);
	fCacheWindow = new BWindow(frame, helper.String(), B_TITLED_WINDOW, windowFlags);
	fCacheView = new CacheView(fCacheWindow->Bounds(), fCacheBitmap);
	fCacheWindow->AddChild(fCacheView);
	fCacheWindow->Show();
#  endif
}

// destructor
RenderThread::LayerBitmap::~LayerBitmap()
{
}

// InitCheck
status_t
RenderThread::LayerBitmap::InitCheck() const
{
	if (!fLayer)
		return B_NO_INIT;
	status_t ret = fLayerBitmap.InitCheck();
	if (ret < B_OK)
		return ret;
#if USE_CACHING
	ret = fCacheBitmap.InitCheck();
	if (ret < B_OK)
		return ret;
#endif
	return B_OK;
}

// Render
BRect
RenderThread::LayerBitmap::Render(BRect area, int32 lowestChangedObject)
{
#if USE_CACHING
	return fLayer->Render(area, lowestChangedObject,
		&fLayerBitmap, &fCacheBitmap, fValidCache, fCacheLevel);
#else
	BRegion dummyRegion;
	int32 dummyLevel;
	return fLayer->Render(area, lowestChangedObject,
		&fLayerBitmap, NULL, dummyRegion, dummyLevel);
#endif
}

// #pragma mark -

// constructor
RenderThread::RenderThread(RenderManager* manager, int32 index)
	: BLooper("render thread")
	, fRenderManager(manager)
	, fThreadIndex(index)
	, fBitmap(new BBitmap(manager->Bounds(), 0, B_RGBA32))
{
}

// destructor
RenderThread::~RenderThread()
{
	int32 count = fLayerBitmaps.CountItems();
	for (int32 i = 0; i < count; i++)
		delete (LayerBitmap*)fLayerBitmaps.ItemAtFast(i);
	delete fBitmap;
}

// MessageReceived
void
RenderThread::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_RENDER: {
			_Render();
			break;
		}
		default:
			BLooper::MessageReceived(message);
			break;
	}
}

// #pragma mark -

// Render
void
RenderThread::Render()
{
	BMessage message(MSG_RENDER);
	PostMessage(&message);
}

// #pragma mark -

// _Render
void
RenderThread::_Render()
{
	_RecursiveRender(fRenderManager->Snapshot());
	fRenderManager->RenderThreadDone(fThreadIndex);
}

// _RecursiveRender
void
RenderThread::_RecursiveRender(LayerSnapshot* layer)
{
	// make sure all the sub layers and their sub-sub layers are rendered first
	int32 count = layer->CountObjects();
	for (int32 i = 0; i < count; i++) {
		LayerSnapshot* childLayer
			= dynamic_cast<LayerSnapshot*>(layer->ObjectAtFast(i));
		if (childLayer)
			_RecursiveRender(childLayer);
	}

	LayerBitmap* bitmap = _LayerBitmapFor(layer);
	if (!bitmap)
		return;

	BRect area;
	int32 lowestChangedObject;

	if (!fRenderManager->GetDirtyInfoFor(fThreadIndex, layer->Layer(),
			area, lowestChangedObject)) {
		// this layer is clean
//printf("layer %p is clean\n", layer->Layer());
		return;
	}

//printf("rendering layer %p BRect(%.1f, %.1f, %.1f, %.1f)\n", layer->Layer(),
//	area.left, area.top, area.right, area.bottom);

	// TODO: this is not correct, since we need to blend from the
	// bottom layer always!
	// a dirty area in a sub-layer will always be dirty in the
	// parent layer as well... so we could use this to only do this
	// if we are down at the bottom with the recursive rendering,
	// additionally the RenderManager could use this to only trigger rendering,
	// if the notification for the bottom layer has arrived after any notifications
	// of the sub-layers

	BRect visuallyChangedArea = bitmap->Render(area, lowestChangedObject);

	// transfer the final visually changed area to the display bitmap
	visuallyChangedArea = visuallyChangedArea & fBitmap->Bounds();
	clear_area(fBitmap, (rgb_color){ 255, 255, 255, 255 },
		visuallyChangedArea);
	blend_area(bitmap->Bitmap(), fBitmap, visuallyChangedArea);

	fRenderManager->TransferClean(fBitmap, visuallyChangedArea);
}

// _LayerSnapshotForLayer
LayerSnapshot*
RenderThread::_LayerSnapshotForLayer(LayerSnapshot* snapshot, Layer* layer)
{
	if (snapshot->Original() == layer)
		return snapshot;
	int32 count = snapshot->CountObjects();
	for (int32 i = 0; i < count; i++) {
		ObjectSnapshot* object = snapshot->ObjectAtFast(i);
		LayerSnapshot* subLayer = dynamic_cast<LayerSnapshot*>(object);
		if (subLayer)
			subLayer = _LayerSnapshotForLayer(subLayer, layer);
		if (subLayer)
			return subLayer;
	}
	return NULL;
}

// _LayerBitmapFor
RenderThread::LayerBitmap*
RenderThread::_LayerBitmapFor(LayerSnapshot* layer)
{
	int32 count = fLayerBitmaps.CountItems();
	for (int32 i = 0; i < count; i++) {
		LayerBitmap* l = (LayerBitmap*)fLayerBitmaps.ItemAtFast(i);
		if (l->Layer() == layer)
			return l;
	}

	LayerBitmap* bitmap = new (nothrow) LayerBitmap(layer);
	if (!bitmap || bitmap->InitCheck() != B_OK
		|| !fLayerBitmaps.AddItem(bitmap)) {
		delete bitmap;
		return NULL;
	}
	return bitmap;
}

