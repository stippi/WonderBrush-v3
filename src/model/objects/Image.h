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
								Image();
								Image(RenderBuffer* buffer);
								Image(const Image& other);
	virtual						~Image();

	// Object interface
	virtual	BaseObject*			Clone(CloneContext& context) const;
	virtual	const char*			DefaultName() const;
	virtual	void				AddProperties(PropertyObject* object,
									uint32 flags = 0) const;
	virtual	bool				SetToPropertyObject(
									const PropertyObject* object,
									uint32 flags = 0);

	virtual	ObjectSnapshot*		Snapshot() const;

	virtual	bool				HitTest(const BPoint& canvasPoint);

	// BoundedObject interface
	virtual	BRect				Bounds();

	// Image
			void				SetBuffer(const RenderBufferRef& buffer);
	inline	RenderBuffer*		Buffer() const
									{ return fBuffer.Get(); }

			void				SetInterpolation(uint32 interpolation);
	inline	uint32				Interpolation() const
									{ return fInterpolation; }

			bool				AddListener(ImageListener* listener);
			void				RemoveListener(ImageListener* listener);

private:
			void				_NotifyDeleted();

private:
			RenderBufferRef		fBuffer;
			uint32				fInterpolation;

			BList				fListeners;
};

#endif // IMAGE_H
