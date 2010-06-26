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

#include "RenderEngine.h"


class Layer;
class LayerSnapshot;
class RenderBuffer;
class RenderManager;

class RenderThread {
public:
								RenderThread(RenderManager* manager);
	virtual						~RenderThread();

			status_t			Init();
			thread_id			Run();
			void				WaitForThread();
			void				Render(LayerSnapshot* layer, BRect area,
									double zoomLevel);

private:
	static	status_t			_WorkerLoopEntry(void* data);
			status_t			_WorkerLoop();

			thread_id			fThread;
			RenderManager*		fRenderManager;
			RenderEngine		fEngine;

			RenderBuffer*		fScratchBitmap;
};

#endif // RENDER_THREAD_H
