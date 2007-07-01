/*
 * Copyright 2006, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#include "RenderManager.h"

#include <new>
#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <Message.h>
#include <Messenger.h>
#include <OS.h>

#include "bitmap_support.h"

#include "LayerSnapshot.h"
#include "RenderThread.h"


using std::nothrow;

// constructor
RenderManager::RenderManager(Document* document, const BRect& bounds)
	: Layer::Listener()
	, fDirtyArea(LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN)
	, fCleanArea(LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN)
	, fLowestDirtyObject(LONG_MAX)

	, fDocument(document)
	, fSnapshot(new (nothrow) LayerSnapshot(fDocument->RootLayer()))

	, fRenderThreads(NULL)
	, fRenderThreadCount(0)

	, fRenderingThreads(0)
	, fRenderQueueLock("render queue lock")

	, fBitmapListener(NULL)
{
	fDisplayBitmap[0] = new (nothrow) BBitmap(bounds, 0, B_RGBA32);
	fDisplayBitmap[1] = new (nothrow) BBitmap(bounds, 0, B_RGBA32);

#if 1
	system_info info;
	get_system_info(&info);

	fRenderThreadCount = info.cpu_count;
#else
	fRenderThreadCount = 1;
#endif

	fRenderThreads = new (nothrow) RenderThread*[fRenderThreadCount];
	for (int32 i = 0; i < fRenderThreadCount; i++) {
		fRenderThreads[i] = new (nothrow) RenderThread(this, i);
		fRenderThreads[i]->Run();
	}

	fDocument->RootLayer()->AddListener(this);
	// trigger the first rendering of the document
	AreaInvalidated(fDocument->RootLayer(), bounds, 0);
}

// destructor
RenderManager::~RenderManager()
{
	for (int32 i = 0; i < fRenderThreadCount; i++) {
		fRenderThreads[i]->Lock();
		fRenderThreads[i]->Quit();
	}
	delete[] fRenderThreads;

	delete fDisplayBitmap[0];
	delete fDisplayBitmap[1];

	delete fSnapshot;
	delete fBitmapListener;

	fDocument->RootLayer()->RemoveListener(this);
}

// #pragma mark -

// AreaInvalidated
void
RenderManager::AreaInvalidated(Layer* layer, const BRect& area,
	int32 objectIndex)
{
	// this is a synchronous notification, therefor the document
	// is already properly locked
	_QueueRedraw(area, objectIndex);
}

// #pragma mark -

// Bounds
BRect
RenderManager::Bounds() const
{
	return fDisplayBitmap[0]->Bounds();
}

// SetBitmapListener
void
RenderManager::SetBitmapListener(BMessenger* listener)
{
	if (!fRenderQueueLock.Lock())
		return;

	fBitmapListener = listener;

	fRenderQueueLock.Unlock();
}

// LockDisplay
bool
RenderManager::LockDisplay()
{
	return fRenderQueueLock.Lock();
}

// UnlockDisplay
void
RenderManager::UnlockDisplay()
{
	fRenderQueueLock.Unlock();
}

// DisplayBitmap
const BBitmap*
RenderManager::DisplayBitmap() const
{
	return fDisplayBitmap[0];
}

// BackBitmap
const BBitmap*
RenderManager::BackBitmap() const
{
	return fDisplayBitmap[1];
}

// TransferClean
void
RenderManager::TransferClean(const BBitmap* bitmap, const BRect& area)
{
	// executed in a rendering thread
	// it is ok to copy bitmap contents without holding the
	// lock, since "flipping" is only done by which ever thread
	// happens to be the *last* thread getting hold of the lock
	copy_area(bitmap, BackBitmap(), area);

	// hold the lock in as short a time as possible
	if (!fRenderQueueLock.Lock())
		return;

	fCleanArea = fCleanArea | area;

	fRenderingThreads--;
	if (fRenderingThreads == 0) {
		_BackToDisplay(fCleanArea);
		fCleanArea.Set(LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN);
	}

	fRenderQueueLock.Unlock();
}

// #pragma mark -

// _QueueRedraw
void
RenderManager::_QueueRedraw(const BRect& area, int32 objectIndex)
{
	if (!area.IsValid() || !fRenderQueueLock.Lock())
		return;

	fDirtyArea = (fDirtyArea | area) & Bounds();
	fLowestDirtyObject = min_c(objectIndex, fLowestDirtyObject);

	if (fRenderingThreads > 0) {
//		printf("rendering in progress\n");
		// rendering in progress
	} else {
//		printf("triggering render\n");
		// idle, trigger rendering:
		// sync document and document clone
		fSnapshot->Sync();
		// split dirty area in part for each thread
		// and dispatch rendering messages
		int32 width = fDirtyArea.IntegerWidth();
		int32 height = fDirtyArea.IntegerHeight();
		if (width * height < 40 * 40) {
			// rendering area too small, use just one thread
			fRenderingThreads = 1;
			fRenderThreads[0]->Render(fDirtyArea, fLowestDirtyObject);
		} else {
			fRenderingThreads = fRenderThreadCount;
			if (width > height) {
				// split horizontally
				float left = fDirtyArea.left - 1;
				for (int32 i = 0; i < fRenderThreadCount; i++) {
					BRect part(fDirtyArea);
					part.left = left + 1;
					left = fDirtyArea.left + ((i + 1) * width)
						/ fRenderThreadCount;
					part.right = left;
					fRenderThreads[i]->Render(part, fLowestDirtyObject);
				}
			} else {
				// split vertically
				float top = fDirtyArea.top - 1;
				for (int32 i = 0; i < fRenderThreadCount; i++) {
					BRect part(fDirtyArea);
					part.top = top + 1;
					top = fDirtyArea.top + ((i + 1) * height)
						/ fRenderThreadCount;
					part.bottom = top;
					fRenderThreads[i]->Render(part, fLowestDirtyObject);
				}
			}
		}

		fDirtyArea.Set(LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN);
		fLowestDirtyObject = LONG_MAX;
	}

	fRenderQueueLock.Unlock();
}

// _BackToDisplay
void
RenderManager::_BackToDisplay(const BRect& area)
{
	// done while holding the queue lock
	copy_area(BackBitmap(), DisplayBitmap(), area);
	demultiply_area(DisplayBitmap(), area);

	if (fBitmapListener) {
		BMessage message(MSG_BITMAP_CLEAN);
		message.AddRect("area", area);
		fBitmapListener->SendMessage(&message);
	}

	if (fDirtyArea.IsValid())
		_QueueRedraw(fDirtyArea, fLowestDirtyObject);
}



