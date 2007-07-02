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

RenderManager::layer_dirty_info::layer_dirty_info()
{
	dirtyArea[0] = dirtyArea[1] = BRect(LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN);
	lowestDirtyObject[0] = lowestDirtyObject[1] = LONG_MAX;
}

RenderManager::layer_dirty_info&
RenderManager::layer_dirty_info::operator=(const layer_dirty_info& info)
{
	dirtyArea[0] = info.dirtyArea[0];
	dirtyArea[1] = info.dirtyArea[1];

	lowestDirtyObject[0] = info.lowestDirtyObject[0];
	lowestDirtyObject[1] = info.lowestDirtyObject[1];

	return *this;
}


// #pragma mark -

// constructor
RenderManager::RenderManager(Document* document, const BRect& bounds)
	: Layer::Listener()
	, fCleanArea(LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN)

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

	_RecursiveAddListener(fDocument->RootLayer());
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

	_RecursiveRemoveListener(fDocument->RootLayer());

	DirtyMap::Iterator iterator = fDirtyMap.GetIterator();
	while (iterator.HasNext())
		delete iterator.Next().value;
}

// #pragma mark -

// ObjectAdded
void
RenderManager::ObjectAdded(Layer* layer, Object* object, int32 index)
{
	// adding ourself as listener to layers without locking is
	// ok, since this is a synchronous notification which is executed
	// in the thread that added the layer
	Layer* subLayer = dynamic_cast<Layer*>(object);
	if (subLayer) {
printf("RenderManager::ObjectAdded(%p)\n", subLayer);
		_RecursiveAddListener(subLayer);
	}
}

// ObjectRemoved
void
RenderManager::ObjectRemoved(Layer* layer, Object* object, int32 index)
{
	// see ObjectAdded on why it is ok to add listener without locking
	Layer* subLayer = dynamic_cast<Layer*>(object);
	if (subLayer) {
printf("RenderManager::ObjectRemoved(%p)\n", subLayer);
		_RecursiveRemoveListener(subLayer);
	}
}

// AreaInvalidated
void
RenderManager::AreaInvalidated(Layer* layer, const BRect& area,
	int32 objectIndex)
{
	// this is a synchronous notification, therefor the document
	// is already properly locked
	_QueueRedraw(layer, area, objectIndex);
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

	fRenderQueueLock.Unlock();
}

// RenderThreadDone
void
RenderManager::RenderThreadDone(int32 threadIndex)
{
	// executed in a rendering thread

	if (!fRenderQueueLock.Lock())
		return;

	fRenderingThreads--;
	if (fRenderingThreads == 0) {
		_BackToDisplay(fCleanArea);
		fCleanArea.Set(LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN);
	}

	fRenderQueueLock.Unlock();
}

// GetDirtyInfoFor
bool
RenderManager::GetDirtyInfoFor(int32 threadIndex, const Layer* layer, 
	BRect& dirtyArea, int32& lowestDirtyObject)
{
	// executed in a rendering thread, we need to lock, because
	// items might be put into the hash by another thread
	if (!fRenderQueueLock.Lock())
		return false;

	if (!fDirtyMap.ContainsKey(layer)) {
		fRenderQueueLock.Unlock();
		return false;
	}
	
	layer_dirty_info* info = fDirtyMap.Get(layer);

	dirtyArea = info->dirtyArea[0];
	if (!dirtyArea.IsValid()) {
		fRenderQueueLock.Unlock();
		return false;
	}

	int32 width = dirtyArea.IntegerWidth();
	int32 height = dirtyArea.IntegerHeight();

	if (width > height) {
		// split horizontally
		dirtyArea.left = dirtyArea.left + threadIndex
			* width / fRenderThreadCount;
		dirtyArea.right = dirtyArea.left + width / fRenderThreadCount + 1;
	} else {
		// split vertically
		dirtyArea.top = dirtyArea.top + threadIndex
			* height / fRenderThreadCount;
		dirtyArea.bottom = dirtyArea.top + height / fRenderThreadCount + 1;
	}

	lowestDirtyObject = info->lowestDirtyObject[0];

	fRenderQueueLock.Unlock();
	return true;
}

// PrepareDirtyInfosForNextRender
void
RenderManager::PrepareDirtyInfosForNextRender()
{
	DirtyMap::Iterator iterator = fDirtyMap.GetIterator();
	while (iterator.HasNext()) {
		layer_dirty_info* info = iterator.Next().value;
		info->dirtyArea[0] = info->dirtyArea[1];
		info->dirtyArea[1] = BRect(LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN);
		info->lowestDirtyObject[0] = info->lowestDirtyObject[1];
		info->lowestDirtyObject[1] = LONG_MAX;
	}
}


// #pragma mark -

// _RecursiveAddListener
void
RenderManager::_RecursiveAddListener(Layer* layer, bool invalidate)
{
	// the document is locked and/or this is executed from within
	// a synchronous notification
	int32 count = layer->CountObjects();
	for (int32 i = 0; i < count; i++) {
		Object* object = layer->ObjectAtFast(i);
		Layer* subLayer = dynamic_cast<Layer*>(object);
		if (subLayer)
			_RecursiveAddListener(subLayer, invalidate);
	}

	layer->AddListener(this);

	if (invalidate)
		AreaInvalidated(layer, layer->Bounds(), 0);
}

// _RecursiveRemoveListener
void
RenderManager::_RecursiveRemoveListener(Layer* layer)
{
	// the document is locked and/or this is executed from within
	// a synchronous notification
	int32 count = layer->CountObjects();
	for (int32 i = 0; i < count; i++) {
		Object* object = layer->ObjectAtFast(i);
		Layer* subLayer = dynamic_cast<Layer*>(object);
		if (subLayer)
			_RecursiveRemoveListener(subLayer);
	}

	layer->RemoveListener(this);
}

// #pragma mark -

// _QueueRedraw
void
RenderManager::_QueueRedraw(Layer* layer, const BRect& area, int32 objectIndex)
{
	if (!area.IsValid() || !fRenderQueueLock.Lock())
		return;

	layer_dirty_info* info;
	if (fDirtyMap.ContainsKey(layer)) {
		info = fDirtyMap.Get(layer);
	} else {
		info = new (nothrow) layer_dirty_info();
		if (!info || fDirtyMap.Put(layer, info) < B_OK) {
			delete info;
			printf("RenderManager::_QueueRedraw() - out of memory!\n");
			fRenderQueueLock.Unlock();
			return;
		}
	}

	info->dirtyArea[1] = (info->dirtyArea[1] | area) & Bounds();
	info->lowestDirtyObject[1] = min_c(objectIndex, info->lowestDirtyObject[1]);

	if (fRenderingThreads > 0) {
//		printf("rendering in progress\n");
		// rendering in progress
	} else {
//		printf("triggering render\n");
		// idle, trigger rendering
		_TriggerRender();
	}

	fRenderQueueLock.Unlock();
}

// _HasDirtyLayers
bool
RenderManager::_HasDirtyLayers() const
{
	DirtyMap::Iterator iterator = fDirtyMap.GetIterator();
	while (iterator.HasNext()) {
		layer_dirty_info* info = iterator.Next().value;
		if (info->dirtyArea[1].IsValid())
			return true;
	}
	return false;
}

// _TriggerRender
void
RenderManager::_TriggerRender()
{
	// move the dirty infos to the front
	PrepareDirtyInfosForNextRender();
	// sync document and document clone
	fSnapshot->Sync();
	// split dirty area in part for each thread
	// and dispatch rendering messages
	fRenderingThreads = fRenderThreadCount;
	for (int32 i = 0; i < fRenderingThreads; i++) {
		fRenderThreads[i]->Render();
	}
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

	if (_HasDirtyLayers())
		_TriggerRender();
}



