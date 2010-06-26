/*
 * Copyright 2007-2010, Stephan Aßmus <superstippi@gmx.de>
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
#include "RenderBuffer.h"
#include "RenderManager.h"


using std::nothrow;

// constructor
RenderThread::RenderThread(RenderManager* manager)
	: fThread(-1)
	, fRenderManager(manager)
	, fEngine()
	, fScratchBitmap(NULL)
{
}

// destructor
RenderThread::~RenderThread()
{
	WaitForThread();

	delete fScratchBitmap;
}

// #pragma mark -

status_t
RenderThread::Init()
{
	fThread = spawn_thread(_WorkerLoopEntry, "render thread", B_LOW_PRIORITY,
		this);
	return fThread >= 0 ? B_OK : B_ERROR;
}

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
RenderThread::Render(LayerSnapshot* layer, BRect area, double zoomLevel)
{
//printf("RenderThread::Render(%p, (%f, %f, %f, %f))\n", layer,
//area.left, area.top, area.right, area.bottom);
	// TODO: Move change of zoom elsewhere and check Resize() success
	// there. Otherwise we may access an invalid RenderBuffer here the next
	// time this method is called after resizing.
	BRect zoomedBounds(layer->Bounds());
	zoomedBounds.left = floorf(zoomedBounds.left * zoomLevel);
	zoomedBounds.top = floorf(zoomedBounds.top * zoomLevel);
	zoomedBounds.right = ceilf(zoomedBounds.right * zoomLevel);
	zoomedBounds.bottom = ceilf(zoomedBounds.bottom * zoomLevel);
	if (fScratchBitmap == NULL || fScratchBitmap->Bounds() != zoomedBounds) {
		// Need to resize the bitmap and render everything
//printf("  resizing scratch bitmap\n");
		delete fScratchBitmap;
		fScratchBitmap = new(std::nothrow) RenderBuffer(zoomedBounds);
		if (fScratchBitmap == NULL || !fScratchBitmap->IsValid())
			return;
		area = zoomedBounds;
	}

	BRegion dummyRegion;
	int32 dummyLevel;
	layer->Render(fEngine, area, fScratchBitmap, NULL, dummyRegion,
		dummyLevel);
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

	fThread = B_BAD_THREAD_ID;
	return B_OK;
}
