/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#ifndef OBJECT_CLONE_H
#define OBJECT_CLONE_H

#include <Rect.h>

class BBitmap;
class Object;

class ObjectSnapshot {
 public:
								ObjectSnapshot(const Object* object);
	virtual						~ObjectSnapshot();

	virtual	const Object*		Original() const = 0;
	virtual	bool				Sync();

	virtual	void				PrepareRendering(BRect documentBounds);
	virtual	void				Render(BBitmap* bitmap, BRect area) const;
	virtual	void				ExtendDirtyArea(BRect& area) const;
	virtual	void				RebuildAreaForDirtyArea(BRect& area) const;
									// TODO: could be BRegions...

 private:
			uint32				fChangeCounter;
};

#endif // OBJECT_CLONE_H
