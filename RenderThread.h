/*
 * Copyright 2007, Stephan Aßmus <superstippi@gmx.de>
 * Copyright 2007, Ingo Weinhold <ingo_weinhold@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT License.
 *		
 */
#ifndef RENDER_THREAD_H
#define RENDER_THREAD_H

#include <List.h>
#include <OS.h>
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

			thread_id			Run();
			void				WaitForThread();
			void				Render(LayerSnapshot* layer, BRect area);

 private:
	class LayerBitmap;

	static	status_t			_WorkerLoopEntry(void* data);
			status_t			_WorkerLoop();

			LayerBitmap*		_LayerBitmapFor(LayerSnapshot* layer);

			thread_id			fThread;
			RenderManager*		fRenderManager;
			int32				fThreadIndex;
			BBitmap*			fBitmap;

			BList				fLayerBitmaps;
};

#endif // RENDER_THREAD_H
