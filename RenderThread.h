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

class RenderThread : public BLooper {
 public:
								RenderThread(RenderManager* manager,
									int32 index);
	virtual						~RenderThread();

	virtual	void				MessageReceived(BMessage* message);

	// RenderThread
			void				Render();

 private:
	class LayerBitmap;

			void				_Render();
			void				_RecursiveRender(LayerSnapshot* layer);
			LayerSnapshot*		_LayerSnapshotForLayer(LayerSnapshot* snapshot,
									Layer* layer);
			LayerBitmap*		_LayerBitmapFor(LayerSnapshot* layer);

			RenderManager*		fRenderManager;
			int32				fThreadIndex;
			BBitmap*			fBitmap;

			BList				fLayerBitmaps;
};

#endif // RENDER_THREAD_H
