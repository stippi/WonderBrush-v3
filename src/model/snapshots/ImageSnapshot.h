/*
 * Copyright 2010-2015, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef IMAGE_SNAPSHOT_H
#define IMAGE_SNAPSHOT_H

#include <GraphicsDefs.h>

#include "BoundedObjectSnapshot.h"

class Image;

class ImageSnapshot : public BoundedObjectSnapshot {
public:
								ImageSnapshot(const Image* image);
	virtual						~ImageSnapshot();

	virtual	const Object*		Original() const;
	virtual	bool				Sync();

	virtual	void				Render(RenderEngine& engine,
									RenderBuffer* bitmap, BRect area) const;

private:
			const Image*		fOriginal;
			RenderBuffer*		fBuffer;
			uint32				fInterpolation;
};

#endif // IMAGE_SNAPSHOT_H
