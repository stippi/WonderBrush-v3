/*
 * Copyright 2007-2008, Stephan Aßmus <superstippi@gmx.de>
 * Copyright 2007, Ingo Weinhold <ingo_weinhold@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT License.
 *		
 */
#define USE_CACHING 0
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
			BRect				Render(BRect area);

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
RenderThread::LayerBitmap::Render(BRect area)
{
#if USE_CACHING
	return fLayer->Render(area, lowestChangedObject,
		&fLayerBitmap, &fCacheBitmap, fValidCache, fCacheLevel);
#else
	BRegion dummyRegion;
	int32 dummyLevel;
	return fLayer->Render(area, &fLayerBitmap, NULL, dummyRegion, dummyLevel);
#endif
}

// #pragma mark -

// constructor
RenderThread::RenderThread(RenderManager* manager, int32 index)
	: fThread(-1)
	, fRenderManager(manager)
	, fThreadIndex(index)
{
	// TODO: Move to Init() function and check errors!
	fThread = spawn_thread(_WorkerLoopEntry, "render thread", B_LOW_PRIORITY, this);
}

// destructor
RenderThread::~RenderThread()
{
	int32 count = fLayerBitmaps.CountItems();
	for (int32 i = 0; i < count; i++)
		delete (LayerBitmap*)fLayerBitmaps.ItemAtFast(i);
}

// #pragma mark -

// Run
thread_id
RenderThread::Run()
{
	if (fThread < 0)
		return fThread;
	status_t error = resume_thread(fThread);
	return (error == B_OK ? fThread : error);
}

// WaitForThread
void
RenderThread::WaitForThread()
{
	if (fThread >= 0) {
		status_t result;
		while (wait_for_thread(fThread, &result) == B_INTERRUPTED);
	}
}


// Render
//
// Called by the RenderManager, but in our own thread
// (_WorkerLoop() -> RenderManager::DoNextRenderJob() -> Render()).
void
RenderThread::Render(LayerSnapshot* layer, BRect area)
{
//printf("RenderThread::Render(%p, (%f, %f, %f, %f))\n", layer, area.left, area.top, area.right, area.bottom);
	LayerBitmap* bitmap = _LayerBitmapFor(layer);
	if (!bitmap)
		return;

//printf("rendering layer %p BRect(%.1f, %.1f, %.1f, %.1f)\n", layer->Layer(),
//	area.left, area.top, area.right, area.bottom);

	bitmap->Render(area);
}

// #pragma mark -

// _WorkerLoopEntry
status_t
RenderThread::_WorkerLoopEntry(void* data)
{
	return static_cast<RenderThread*>(data)->_WorkerLoop();
}

// _WorkerLoop
status_t
RenderThread::_WorkerLoop()
{
	while (fRenderManager->DoNextRenderJob(this));

	return B_OK;
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

