/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#ifndef LAYER_SNAPSHOT_H
#define LAYER_SNAPSHOT_H


#include <List.h>

#include "ObjectSnapshot.h"


class BBitmap;
class BRegion;
class Layer;
class ObjectSnapshot;

class LayerSnapshot : public ObjectSnapshot {
 public:
								LayerSnapshot(const Layer* layer);
	virtual						~LayerSnapshot();

	// ObjectSnapshot interface
	virtual	const Object*		Original() const;
	virtual	bool				Sync();

	virtual	void				Render(BBitmap* bitmap, BRect area) const;

	// LayerSnapshot
			BRect				Bounds() const;

			BRect				Render(BRect area, int32 lowestObject,
									BBitmap* bitmap,
									BBitmap* cacheBitmap,
									BRegion& validCacheRegion,
									int32& cacheLevel) const;

			ObjectSnapshot*		ObjectAt(int32 index) const;
			ObjectSnapshot*		ObjectAtFast(int32 index) const;
			int32				CountObjects() const;

 private:
			void				_Sync();
			void				_MakeEmpty();

			const Layer*		fOriginal;
			BList				fObjects;
			BBitmap*			fBitmap;
};

#endif // LAYER_SNAPSHOT_H
