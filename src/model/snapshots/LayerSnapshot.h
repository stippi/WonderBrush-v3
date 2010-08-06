/*
 * Copyright 2007-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef LAYER_SNAPSHOT_H
#define LAYER_SNAPSHOT_H

#include <List.h>

#include "BlendingMode.h"
#include "ObjectSnapshot.h"

class RenderBuffer;
class BRegion;
class Layer;
class ObjectSnapshot;

class LayerSnapshot : public ObjectSnapshot {
public:
								LayerSnapshot(const ::Layer* layer);
	virtual						~LayerSnapshot();

	// ObjectSnapshot interface
	virtual	const Object*		Original() const;
	virtual	bool				Sync();

	virtual	void				Layout(LayoutContext& context, uint32 flags);

	virtual	void				Render(RenderEngine& engine,
									RenderBuffer* bitmap, BRect area) const;

	// LayerSnapshot
	inline	const ::Layer*		Layer() const
									{ return fOriginal; }

			RenderBuffer*		Bitmap() const	 { return fBitmap; }
			BRect				Bounds() const;

			BRect				Render(RenderEngine& engine, BRect area,
									RenderBuffer* bitmap,
									RenderBuffer* cacheBitmap,
									BRegion& validCacheRegion,
									int32& cacheLevel) const;

			ObjectSnapshot*		ObjectAt(int32 index) const;
			ObjectSnapshot*		ObjectAtFast(int32 index) const;
			int32				CountObjects() const;

 private:
			void				_Sync();
			void				_MakeEmpty();

			const ::Layer*		fOriginal;
			BList				fObjects;
			BRect				fBounds;
			RenderBuffer*		fBitmap;
			uint8				fGlobalAlpha;
			::BlendingMode		fBlendingMode;
};

#endif // LAYER_SNAPSHOT_H
