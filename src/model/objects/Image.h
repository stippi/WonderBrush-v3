/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef IMAGE_H
#define IMAGE_H

#include <List.h>

#include "BoundedObject.h"

class Image;
class RenderBuffer;

class ImageListener {
public:
								ImageListener();
	virtual						~ImageListener();

	virtual	void				Deleted(Image* image);
};

class Image : public BoundedObject {
public:
								Image(RenderBuffer* buffer);
	virtual						~Image();

	// Object interface
	virtual	ObjectSnapshot*		Snapshot() const;

	virtual	const char*			DefaultName() const;

	virtual	bool				HitTest(const BPoint& canvasPoint);

	// BoundedObject interface
	virtual	BRect				Bounds();

	// Image
	inline	RenderBuffer*		Buffer() const
									{ return fBuffer; }

			bool				AddListener(ImageListener* listener);
			void				RemoveListener(ImageListener* listener);

private:
			void				_NotifyDeleted();

private:
			RenderBuffer*		fBuffer;

			BList				fListeners;
};

#endif // IMAGE_H
