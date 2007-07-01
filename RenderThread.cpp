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
			BRect area;
			int32 lowestChangedObject;
			if (message->FindRect("area", &area) == B_OK
				&& message->FindInt32("index", &lowestChangedObject) == B_OK)
				_Render(area, lowestChangedObject);
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
RenderThread::Render(const BRect& area, int32 lowestChangedObject)
{
	BMessage message(MSG_RENDER);
	message.AddRect("area", area);
	message.AddInt32("index", lowestChangedObject);
	PostMessage(&message);
}

// #pragma mark -

// _Render
void
RenderThread::_Render(BRect area, int32 lowestChangedObject)
{
	area = area & fBitmap->Bounds();
	if (!area.IsValid()) {
		fRenderManager->TransferClean(fBitmap, area);
		return;
	}
//printf("\nrequest: "); area.PrintToStream();

	LayerSnapshot* layer = fRenderManager->Snapshot();

	BRect visuallyChangedArea = _RecursiveRender(layer, area);

	// transfer the final visually changed area to the display bitmap
	visuallyChangedArea = visuallyChangedArea & fBitmap->Bounds();
	if (LayerBitmap* bitmap = _LayerBitmapFor(layer)) {
		clear_area(fBitmap, (rgb_color){ 255, 255, 255, 255 },
			visuallyChangedArea);
		blend_area(bitmap->Bitmap(), fBitmap, visuallyChangedArea);
	}

	fRenderManager->TransferClean(fBitmap, visuallyChangedArea);
}

// _RecursiveRender
BRect
RenderThread::_RecursiveRender(LayerSnapshot* layer, BRect area)
{
	// make sure all the sub layers and their sub-sub layers are rendered first
	BRect visuallyChanged(LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN);
	int32 count = layer->CountObjects();
	for (int32 i = 0; i < count; i++) {
		LayerSnapshot* childLayer
			= dynamic_cast<LayerSnapshot*>(layer->ObjectAtFast(i));
		if (childLayer)
			visuallyChanged = visuallyChanged | _RecursiveRender(childLayer, area);
	}

	LayerBitmap* bitmap = _LayerBitmapFor(layer);
	return visuallyChanged | bitmap->Render(area, 0);
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

