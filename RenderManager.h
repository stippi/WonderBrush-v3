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
#include "LayerSnapshot.h"

class BBitmap;
class BMessenger;
class Document;
class LayerSnapshot;
class RenderThread;

enum {
	MSG_BITMAP_CLEAN = 'bcln'
};


// RenderManager
class RenderManager : Layer::Listener {
 public:
								RenderManager(Document* document);
	virtual						~RenderManager();

	// Layer::Listener interface
	virtual	void				ObjectAdded(Layer* layer, Object* object,
									int32 index);
	virtual	void				ObjectRemoved(Layer* layer, Object* object,
									int32 index);

	virtual	void				AreaInvalidated(Layer* layer,
									const BRect& area);

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

			void				PrepareDirtyInfosForNextRender();

			bool				LockRenderInfo();
			void				UnlockRenderInfo();

			bool				DoNextRenderJob(RenderThread* thread);
			void				WakeUpRenderThreads();

 private:
			typedef HashMap<HashKey32<const Layer*>, BRect*> DirtyMap;
			struct RenderInfo;
			class LayerSnapshotVisitor;
			class RenderInfoInitVisitor;

			friend class RenderInfoInitVisitor;

			void				_RecursiveAddListener(Layer* layer,
									bool invalidate = true);
			void				_RecursiveRemoveListener(Layer* layer);

			void				_QueueRedraw(Layer* layer, const BRect& area);
			bool				_HasDirtyLayers() const;
			void				_TriggerRender();
			void				_BackToDisplay(const BRect& area);

			void				_ClearDirtyMap(DirtyMap* map);

			bool				_ResizeRenderInfos(int32 size);
			void				_TraverseLayerSnapshots(
									LayerSnapshotVisitor* visitor,
									LayerSnapshot* layer,
									int32& count, int32 previousSibling);

			void				_AllRenderThreadsDone();

private:
			BBitmap*			fDisplayBitmap[2];

			BRect				fCleanArea;

			DirtyMap*			fDocumentDirtyMap;
			DirtyMap*			fSnapshotDirtyMap;

			Document*			fDocument;
			LayerSnapshot*		fSnapshot;

			RenderThread**		fRenderThreads;
			int32				fRenderThreadCount;

			RenderInfo*			fRenderInfos;
			int32				fRenderInfoCount;
			int32				fRenderInfoCapacity;
			int32				fCurrentRenderInfo;

			sem_id				fWaitingRenderThreadsSem;
			int32				fWaitingRenderThreadCount;

			BLocker				fRenderQueueLock;

			BMessenger*			fBitmapListener;
};

// RenderInfoLocking
class RenderInfoLocking {
public:
	inline bool Lock(RenderManager *lockable)
	{
		return lockable->LockRenderInfo();
	}

	inline void Unlock(RenderManager *lockable)
	{
		lockable->UnlockRenderInfo();
	}
};

typedef AutoLocker<RenderInfoLocking> RenderInfoLocker;


#endif // RENDER_MANAGER_H
