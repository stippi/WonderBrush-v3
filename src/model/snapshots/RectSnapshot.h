/*
 * Copyright 2007-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef RECT_SNAPSHOT_H
#define RECT_SNAPSHOT_H

#include <GraphicsDefs.h>

#include "StyleableSnapshot.h"

class Rect;

class RectSnapshot : public StyleableSnapshot {
public:
								RectSnapshot(const Rect* rect);
	virtual						~RectSnapshot();

	virtual	const Object*		Original() const;
	virtual	bool				Sync();

	virtual	void				Render(RenderEngine& engine,
									RenderBuffer* bitmap, BRect area) const;

private:
			const Rect*			fOriginal;
			BRect				fArea;
			double				fRoundCornerRadius;
};

#endif // RECT_SNAPSHOT_H
