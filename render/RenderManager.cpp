/*
 * Copyright 2006-2008, Stephan Aßmus <superstippi@gmx.de>
 * Copyright 2007, Ingo Weinhold <ingo_weinhold@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT License.
 *		
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


enum {
	MIN_AREA_PER_THREAD = 2000
};


// RenderInfo
struct RenderManager::RenderInfo {
	LayerSnapshot*		layer;
	int32				parent;
	BRect*				dirtyArea;
	int32				dirtySubLayers;
	int32				splitCount;
	int32				splitCountStarted;
	int32				splitCountDone;
	bool				splitHorizontally;
};


// LayerSnapshotVisitor
class RenderManager::LayerSnapshotVisitor {
public:
	virtual ~LayerSnapshotVisitor()
	{
	}

	virtual void Visit(LayerSnapshot* layer, int32 index,
		int32 lastChildIndex, int32 previousSiblingIndex) = 0;
};


// RenderInfoInitVisitor
class RenderManager::RenderInfoInitVisitor : public LayerSnapshotVisitor {
public:
	RenderInfoInitVisitor(RenderManager* manager)
		: fManager(manager)
	{
	}

	virtual void Visit(LayerSnapshot* layer, int32 index,
		int32 lastChildIndex, int32 previousSiblingIndex)
	{
		RenderInfo& info = fManager->fRenderInfos[index];
		info.layer = layer;
		info.dirtyArea = fManager->fSnapshotDirtyMap->Get(layer->Layer());
		if (info.dirtyArea) {
			// determine split strategy
			int32 width = info.dirtyArea->IntegerWidth() + 1;
			int32 height = info.dirtyArea->IntegerHeight() + 1;
			int64 area = int64(width) * height;
			int64 splitCount = area / MIN_AREA_PER_THREAD;
			splitCount = min_c(splitCount, fManager->fRenderThreadCount);

			if (splitCount > 1)
				info.splitCount = splitCount;
			else
				info.splitCount = 1;

			info.splitCountStarted = 0;
			info.splitCountDone = 0;
			info.splitHorizontally = (width > height);
		} else {
			info.dirtySubLayers = 0;
			info.splitCount = 0;
		}

		// We initialize info.parent with the index of our previous sibling,
		// thus building a linked list of siblings. The Visit() for our parent
		// (which is called later (that's why the parent index is not known
		// yet)) will traverse this list and update info.parent with the correct
		// value. The root layer doesn't have a sibling, so info.parent will be
		// set to and remain -1.
		info.parent = previousSiblingIndex;

		// count the dirty child layers and update their info.parent
		int32 dirtyChildCount = 0;
		int32 childIndex = lastChildIndex;
		while (childIndex >= 0) {
			RenderInfo& childInfo = fManager->fRenderInfos[childIndex];
			childIndex = childInfo.parent;
			childInfo.parent = index;
			if (childInfo.dirtyArea)
				dirtyChildCount++;
		}

		info.dirtySubLayers = dirtyChildCount;
	}

private:
	RenderManager*	fManager;
};


// #pragma mark -

// constructor
RenderManager::RenderManager(Document* document)
	: Layer::Listener()
	, fCleanArea(LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN)

	, fDocumentDirtyMap(new (nothrow) DirtyMap())
	, fSnapshotDirtyMap(new (nothrow) DirtyMap())

	, fDocument(document)
	, fSnapshot(new (nothrow) LayerSnapshot(fDocument->RootLayer()))

	, fInitialLayoutState()
	, fLayoutContext(&fInitialLayoutState)
	, fLayoutDirtyFlags(0)

	, fRenderThreads(NULL)
	, fRenderThreadCount(0)

	, fRenderInfos(NULL)
	, fRenderInfoCount(0)
	, fRenderInfoCapacity(0)
	, fCurrentRenderInfo(0)

	, fWaitingRenderThreadsSem(-1)
	, fWaitingRenderThreadCount(0)

	, fRenderQueueLock("render queue lock")

	, fBitmapListener(NULL)

	, fLastRenderStartTime(-1)
{
	fDisplayBitmap[0] = NULL;
	fDisplayBitmap[1] = NULL;
}

// Init
status_t
RenderManager::Init()
{
// TODO: Move everything to Init() method and check errors!
	fDisplayBitmap[0] = new (nothrow) BBitmap(fDocument->Bounds(), 0, B_RGBA32);
	fDisplayBitmap[1] = new (nothrow) BBitmap(fDocument->Bounds(), 0, B_RGBA32);

	if (fDisplayBitmap[0] == NULL || !fDisplayBitmap[0]->IsValid()
		|| fDisplayBitmap[1] == NULL || !fDisplayBitmap[1]->IsValid()
		|| fDocumentDirtyMap == NULL || fSnapshotDirtyMap == NULL) {
		return B_NO_MEMORY;
	}

#if 1
	system_info info;
	get_system_info(&info);

	fRenderThreadCount = info.cpu_count;
#else
	fRenderThreadCount = 1;
#endif

	fWaitingRenderThreadsSem = create_sem(0, "wait for render task");
	if (fWaitingRenderThreadsSem < 0)
		return fWaitingRenderThreadsSem;

	fRenderThreads = new (nothrow) RenderThread*[fRenderThreadCount];
	if (fRenderThreads == NULL)
		return B_NO_MEMORY;

	memset(fRenderThreads, 0, sizeof(RenderThread*) * fRenderThreadCount);

	for (int32 i = 0; i < fRenderThreadCount; i++) {
		fRenderThreads[i] = new (nothrow) RenderThread(this);
		if (fRenderThreads[i] == NULL || fRenderThreads[i]->Init() != B_OK) {
			delete fRenderThreads[i];
			fRenderThreads[i] = NULL;
			return B_NO_MEMORY;
		}
		fRenderThreads[i]->Run();
	}

	_RecursiveAddListener(fDocument->RootLayer());

	return B_OK;
}
// destructor
RenderManager::~RenderManager()
{
	// this will unblock any waiting render threads
	delete_sem(fWaitingRenderThreadsSem);

	for (int32 i = 0; i < fRenderThreadCount; i++)
		delete fRenderThreads[i];
	delete[] fRenderThreads;

	delete fDisplayBitmap[0];
	delete fDisplayBitmap[1];

	delete fSnapshot;
	delete fBitmapListener;

	_RecursiveRemoveListener(fDocument->RootLayer());

	_ClearDirtyMap(fDocumentDirtyMap);
	_ClearDirtyMap(fSnapshotDirtyMap);
	delete fDocumentDirtyMap;
	delete fSnapshotDirtyMap;

	delete[] fRenderInfos;
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
RenderManager::AreaInvalidated(Layer* layer, const BRect& area)
{
	// this is a synchronous notification, therefore the document
	// is already properly locked
	_QueueRedraw(layer, area);
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
	const BBitmap* targetBitmap = BackBitmap();
	clear_area(targetBitmap, (rgb_color){ 255, 255, 255, 255 }, area);
	blend_area(bitmap, targetBitmap, area);

	// hold the lock in as short a time as possible
	if (!fRenderQueueLock.Lock())
		return;

	fCleanArea = fCleanArea | area;

	fRenderQueueLock.Unlock();
}

// PrepareDirtyInfosForNextRender
void
RenderManager::PrepareDirtyInfosForNextRender()
{
	_ClearDirtyMap(fSnapshotDirtyMap);
	DirtyMap* map = fSnapshotDirtyMap;
	fSnapshotDirtyMap = fDocumentDirtyMap;
	fDocumentDirtyMap = map;
}

// LockRenderInfo
bool
RenderManager::LockRenderInfo()
{
	return fRenderQueueLock.Lock();
}

// UnlockRenderInfo
void
RenderManager::UnlockRenderInfo()
{
	fRenderQueueLock.Unlock();
}

// DoNextRenderJob
bool
RenderManager::DoNextRenderJob(RenderThread* thread)
{
	AutoLocker<BLocker> locker(fRenderQueueLock);
//printf("RenderManager::DoNextRenderJob(%p)\n", thread);

	// iterate through the render infos and find the next open task
	while (fCurrentRenderInfo < fRenderInfoCount) {
		RenderInfo& info = fRenderInfos[fCurrentRenderInfo];
		if (info.dirtyArea && info.dirtySubLayers == 0
			&& info.splitCountStarted < info.splitCount) {
			// found one -- get the area to render
			int32 index = info.splitCountStarted;
			BRect dirtyArea = *info.dirtyArea;
			int32 width = dirtyArea.IntegerWidth() + 1;
			int32 height = dirtyArea.IntegerHeight() + 1;

			if (info.splitHorizontally) {
				// split horizontally
				dirtyArea.right = dirtyArea.left
					+ (index + 1) * width / info.splitCount - 1;
				dirtyArea.left = dirtyArea.left
					+ index * width / info.splitCount;
			} else {
				// split vertically
				dirtyArea.bottom = dirtyArea.top
					+ (index + 1) * height / info.splitCount - 1;
				dirtyArea.top = dirtyArea.top
					+ index * height / info.splitCount;
			}

//printf("  -> rendering part %ld/%ld of render info %ld (/%ld)\n", index + 1,
//info.splitCount, fCurrentRenderInfo, fRenderInfoCount);
			info.splitCountStarted++;

			// render
			locker.Unlock();

			thread->Render(info.layer, dirtyArea);

			// If we rendered something for the root layer, we transfer it to
			// the display bitmap.
			if (info.layer == fSnapshot)
				TransferClean(fSnapshot->Bitmap(), dirtyArea);

			locker.Lock();

			// post processing
			info.splitCountDone++;
			if (info.splitCountDone == info.splitCount) {
				// We finished the last missing split area. This layer is clean,
				// now. Update the parent.
				if (info.parent >= 0) {
					RenderInfo& parentInfo = fRenderInfos[info.parent];
					parentInfo.dirtySubLayers--;
					if (parentInfo.dirtySubLayers == 0) {
						// The parent layer has got no more dirty sublayers. It
						// can be rendered now.
						if (fCurrentRenderInfo > info.parent)
							fCurrentRenderInfo = info.parent;

						// If any threads are waiting, they might find a task
						// now.
						WakeUpRenderThreads();
					}
				}
			}

			// another job well done
			return true;
		}

		// no luck yet
		fCurrentRenderInfo++;
	}

	// There's nothing we can do at the moment.
	fWaitingRenderThreadCount++;
	if (fWaitingRenderThreadCount == fRenderThreadCount) {
		// All the other threads are waiting too, which means everything has
		// been rendered.
		_AllRenderThreadsDone();
	}

//printf("  -> nothing to do ATM, %ld/%ld threads waiting\n", fWaitingRenderThreadCount, fRenderThreadCount);
	locker.Unlock();

	status_t error;
	do {
		error = acquire_sem(fWaitingRenderThreadsSem);
	} while (error == B_INTERRUPTED);

	// If not OK, the semaphore has been destroyed. Our signal to quit.
	return (error == B_OK);
}

// WakeUpRenderThreads
void
RenderManager::WakeUpRenderThreads()
{
	AutoLocker<BLocker> locker(fRenderQueueLock);
	if (fWaitingRenderThreadCount > 0) {
		release_sem_etc(fWaitingRenderThreadsSem, fWaitingRenderThreadCount,
			B_DO_NOT_RESCHEDULE);
		fWaitingRenderThreadCount = 0;
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
		AreaInvalidated(layer, layer->Bounds());
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
RenderManager::_QueueRedraw(Layer* layer, BRect area)
{
	if (!area.IsValid() || !fRenderQueueLock.Lock())
		return;
//printf("RenderManager::_QueueRedraw(%p, (%f, %f, %f, %f))\n", layer, area.left, area.top, area.right, area.bottom);

	area.left = floorf(area.left);
	area.top = floorf(area.top);
	area.right = ceilf(area.right);
	area.bottom = ceilf(area.bottom);

	BRect* info;
	if (fDocumentDirtyMap->ContainsKey(layer)) {
		info = fDocumentDirtyMap->Get(layer);
	} else {
		info = new (nothrow) BRect(LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN);
		if (!info || fDocumentDirtyMap->Put(layer, info) < B_OK) {
			delete info;
			printf("RenderManager::_QueueRedraw() - out of memory!\n");
			fRenderQueueLock.Unlock();
			return;
		}
	}

	*info = *info | (area & Bounds());

	if (fWaitingRenderThreadCount < fRenderThreadCount) {
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
	return (fDocumentDirtyMap->Size() > 0);
}

// _TriggerRender
void
RenderManager::_TriggerRender()
{
//printf("RenderManager::_TriggerRender()\n");
	fLastRenderStartTime = system_time();

	// move the dirty infos to the front
	PrepareDirtyInfosForNextRender();

	// sync document and document clone
	fSnapshot->Sync();

	// do layout pass
	fSnapshot->Layout(fLayoutContext, fLayoutDirtyFlags);

	// count sublayers
	int32 count = 0;
	_TraverseLayerSnapshots(NULL, fSnapshot, count, -1);

	_ResizeRenderInfos(count);

	// prepare render infos
	RenderInfoInitVisitor visitor(this);
	count = 0;
	_TraverseLayerSnapshots(&visitor, fSnapshot, count, -1);
	fCurrentRenderInfo = 0;

	// and go
	WakeUpRenderThreads();
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
}

// _ClearDirtyMap
void
RenderManager::_ClearDirtyMap(DirtyMap* map)
{
	DirtyMap::Iterator iterator = map->GetIterator();
	while (iterator.HasNext())
		delete iterator.Next().value;
	map->Clear();
}

// _ResizeRenderInfos
bool
RenderManager::_ResizeRenderInfos(int32 size)
{
	if (size > fRenderInfoCapacity) {
		// delete old array
		delete[] fRenderInfos;
		fRenderInfoCapacity = 0;

		// create new one
		fRenderInfos = new(nothrow) RenderInfo[size];
		if (!fRenderInfos)
			return false;
		fRenderInfoCapacity = size;
	}

	fRenderInfoCount = size;
	return true;
}

// _TraverseLayerSnapshots
/*!	Traverses the layer tree in post order.
	\param visitor The visitor to notify.
	\param layer The layer tree to be visited recursively. This layer will be
		   visited after all its descendants.
	\param count In/out parameter incremented whenever a layer has been visited.
	\param previousSibling The index of the previous sibling visited. -1 if this
		   is the first sibling.
*/
void
RenderManager::_TraverseLayerSnapshots(LayerSnapshotVisitor* visitor,
	LayerSnapshot* layer, int32& count, int32 previousSibling)
{
//printf("RenderManager::_TraverseLayerSnapshots(%p, %p, %ld, %ld)\n", visitor, layer, count, previousSibling);
	// traverse sublayers
	int32 lastChild = -1;
	int32 objectCount = layer->CountObjects();
	for (int32 i = 0; i < objectCount; i++) {
		LayerSnapshot* child = dynamic_cast<LayerSnapshot*>(layer->ObjectAt(i));
		if (child) {
			_TraverseLayerSnapshots(visitor, child, count, lastChild);
			lastChild = count - 1;
		}
	}

	// visit this layer
	if (visitor)
		visitor->Visit(layer, count, lastChild, previousSibling);
	count++;
}

// _AllRenderThreadsDone
//
// fRenderQueueLock must be locked.
void
RenderManager::_AllRenderThreadsDone()
{
	// executed in a rendering thread
	if (fCleanArea.IsValid()) {
		_BackToDisplay(fCleanArea);
		fCleanArea.Set(LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN);
	}

	if (fLastRenderStartTime > 0)
		printf("render pass: %lld\n", system_time() - fLastRenderStartTime);

	if (_HasDirtyLayers())
		_TriggerRender();
}

