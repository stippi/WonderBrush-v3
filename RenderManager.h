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

 private:
			void				_QueueRedraw(const BRect& area,
									int32 objectIndex);

			void				_BackToDisplay(const BRect& area);

			BBitmap*			fDisplayBitmap[2];
			BRect				fDirtyArea;
			BRect				fCleanArea;
			int32				fLowestDirtyObject;

			Document*			fDocument;
			LayerSnapshot*		fSnapshot;

			RenderThread**		fRenderThreads;
			int32				fRenderThreadCount;

			vint32				fRenderingThreads;
			BLocker				fRenderQueueLock;

			BMessenger*			fBitmapListener;
};

#endif // RENDER_MANAGER_H
