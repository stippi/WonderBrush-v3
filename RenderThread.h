/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#ifndef RENDER_THREAD_H
#define RENDER_THREAD_H

#include <List.h>
#include <Looper.h>
#include <Region.h>


class BBitmap;
class Layer;
class LayerSnapshot;
class RenderManager;

class RenderThread {
 public:
								RenderThread(RenderManager* manager,
									int32 index);
	virtual						~RenderThread();

	// RenderThread
			thread_id			Run();
			void				WaitForThread();
			void				Render(LayerSnapshot* layer, BRect area);

 private:
	class LayerBitmap;

	static	status_t			_WorkerLoopEntry(void* data);
			status_t			_WorkerLoop();

			void				_Render();
			void				_RecursiveRender(LayerSnapshot* layer);
			LayerSnapshot*		_LayerSnapshotForLayer(LayerSnapshot* snapshot,
									Layer* layer);
			LayerBitmap*		_LayerBitmapFor(LayerSnapshot* layer);

			thread_id			fThread;
			RenderManager*		fRenderManager;
			int32				fThreadIndex;
			BBitmap*			fBitmap;

			BList				fLayerBitmaps;
};

#endif // RENDER_THREAD_H
