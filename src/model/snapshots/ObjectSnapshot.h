/*
 * Copyright 2007 - 2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef OBJECT_SNAPSHOT_H
#define OBJECT_SNAPSHOT_H

#include <Rect.h>

#include "LayoutContext.h"
#include "LayoutState.h"
#include "Transformable.h"

class BBitmap;
class Object;
class RenderEngine;

class ObjectSnapshot : public Transformable {
public:
								ObjectSnapshot(const Object* object);
	virtual						~ObjectSnapshot();

	virtual	const Object*		Original() const = 0;
	virtual	bool				Sync();

	// Method is called prior to rendering. Use this to compute and cache any
	// properties, like the global transformation effective for this object.
	virtual	void				Layout(LayoutContext& context, uint32 flags);

	// Method is called in a rendering thread and needs to do it's own locking.
	// Any number of other threads may call it, but it should be executed only
	// once. This method may be more expensive than Layout(), therefor it is
	// allowed to run in parallel to other render threads.
	virtual	void				PrepareRendering(BRect documentBounds);
	virtual	void				Render(RenderEngine& engine, BBitmap* bitmap,
									BRect area) const;

	virtual	void				RebuildAreaForDirtyArea(BRect& area) const;
									// TODO: could be BRegions...

	inline	const LayoutState&	LayoutedState() const
									{ return fLayoutedState; }

private:
			uint32				fChangeCounter;
			LayoutState			fLayoutedState;
};

#endif // OBJECT_SNAPSHOT_H
