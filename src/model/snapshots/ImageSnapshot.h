/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef IMAGE_SNAPSHOT_H
#define IMAGE_SNAPSHOT_H

#include <GraphicsDefs.h>

#include "ObjectSnapshot.h"

class Image;

class ImageSnapshot : public ObjectSnapshot {
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
};

#endif // IMAGE_SNAPSHOT_H
