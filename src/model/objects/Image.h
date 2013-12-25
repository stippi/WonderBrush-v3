/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef IMAGE_H
#define IMAGE_H

#include <List.h>

#include "BoundedObject.h"
#include "Referenceable.h"

class Image;
class RenderBuffer;

typedef Reference<RenderBuffer> RenderBufferRef;

class ImageListener {
public:
								ImageListener();
	virtual						~ImageListener();

	virtual	void				Deleted(Image* image);
};

class Image : public BoundedObject {
public:
								Image(RenderBuffer* buffer);
								Image(const Image& other);
	virtual						~Image();

	// Object interface
	virtual	BaseObject*			Clone(ResourceResolver& resolver) const;

	virtual	ObjectSnapshot*		Snapshot() const;

	virtual	const char*			DefaultName() const;

	virtual	bool				HitTest(const BPoint& canvasPoint);

	// BoundedObject interface
	virtual	BRect				Bounds();

	// Image
	inline	RenderBuffer*		Buffer() const
									{ return fBuffer.Get(); }

			bool				AddListener(ImageListener* listener);
			void				RemoveListener(ImageListener* listener);

private:
			void				_NotifyDeleted();

private:
			RenderBufferRef		fBuffer;

			BList				fListeners;
};

#endif // IMAGE_H
