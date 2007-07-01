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
class RenderManager;
class LayerSnapshot;

class RenderThread : public BLooper {
 public:
								RenderThread(RenderManager* manager,
									int32 index);
	virtual						~RenderThread();

	virtual	void				MessageReceived(BMessage* message);

	// RenderThread
			void				Render(const BRect& area,
									int32 lowestChangedObject);

 private:
	class LayerBitmap;

			void				_Render(BRect area, int32 lowestChangedObject);
			BRect				_RecursiveRender(LayerSnapshot* layer, BRect area);
			LayerBitmap*		_LayerBitmapFor(LayerSnapshot* layer);

			RenderManager*		fRenderManager;
			BBitmap*			fBitmap;

			BList				fLayerBitmaps;
};

#endif // RENDER_THREAD_H
