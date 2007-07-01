/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#ifndef RECT_SNAPSHOT_H
#define RECT_SNAPSHOT_H

#include <GraphicsDefs.h>
#include "ObjectSnapshot.h"

class Rect;

class RectSnapshot : public ObjectSnapshot {
 public:
								RectSnapshot(const Rect* rect);
	virtual						~RectSnapshot();

	virtual	const Object*		Original() const;
	virtual	bool				Sync();

	virtual	void				Render(BBitmap* bitmap, BRect area) const;

 private:
			const Rect*			fOriginal;
			BRect				fArea;
			rgb_color			fColor;
};

#endif // RECT_SNAPSHOT_H
