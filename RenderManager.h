/*
 * Copyright 2006, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H

#include <Locker.h>
#include <Rect.h>

#include "Document.h"
#include "HashMap.h"
#include "Layer.h"

class BBitmap;
class BMessenger;
class Document;
class LayerSnapshot;
class RenderThread;

enum {
	MSG_BITMAP_CLEAN = 'bcln'
};

class RenderManager : Layer::Listener {
 public:
								RenderManager(Document* document,
									const BRect& bounds);
	virtual						~RenderManager();

	// Layer::Listener interface
	virtual	void				ObjectAdded(Layer* layer, Object* object,
									int32 index);
	virtual	void				ObjectRemoved(Layer* layer, Object* object,
									int32 index);

	virtual	void				AreaInvalidated(Layer* layer, const BRect& area,
									int32 objectIndex);

	// RenderManager
			LayerSnapshot*		Snapshot() const
									{ return fSnapshot; }

			BRect				Bounds() const;

			void				SetBitmapListener(BMessenger* listener);

			bool				LockDisplay();
			void				UnlockDisplay();
			const BBitmap*		DisplayBitmap() const;
			const BBitmap*		BackBitmap() const;

			void				TransferClean(const BBitmap* bitmap,
									const BRect& area);
			void				RenderThreadDone(int32 threadIndex);

			bool				GetDirtyInfoFor(int32 threadIndex,
									const Layer* layer,
									BRect& dirtyArea,
									int32& lowestDirtyObject);

			void				PrepareDirtyInfosForNextRender();

 private:
			void				_RecursiveAddListener(Layer* layer,
									bool invalidate = true);
			void				_RecursiveRemoveListener(Layer* layer);

			void				_QueueRedraw(Layer* layer, const BRect& area,
									int32 objectIndex);
			bool				_HasDirtyLayers() const;
			void				_TriggerRender();
			void				_BackToDisplay(const BRect& area);

			BBitmap*			fDisplayBitmap[2];

			BRect				fCleanArea;

			struct layer_dirty_info {
									layer_dirty_info();
				layer_dirty_info&	operator=(const layer_dirty_info& info);
				BRect				dirtyArea[2];
				int32				lowestDirtyObject[2];
			};
			typedef HashMap<HashKey32<const Layer*>, layer_dirty_info*> DirtyMap;
			DirtyMap			fDirtyMap;

			Document*			fDocument;
			LayerSnapshot*		fSnapshot;

			RenderThread**		fRenderThreads;
			int32				fRenderThreadCount;

			vint32				fRenderingThreads;
			BLocker				fRenderQueueLock;

			BMessenger*			fBitmapListener;
};

#endif // RENDER_MANAGER_H
