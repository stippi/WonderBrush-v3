/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#ifndef RENDER_BUFFER_H
#define RENDER_BUFFER_H

#include <Rect.h>

class BBitmap;

class RenderBuffer {
 public:
								RenderBuffer(BBitmap* bitmap, BRect area,
									bool adopt);
								RenderBuffer(uint8* buffer,
									uint32 width, uint32 height,
									uint32 bytesPerRow, bool adopt);
	virtual						~RenderBuffer();

			void				Attach(uint8* buffer,
									uint32 width, uint32 height,
									uint32 bytesPerRow,
									bool adopt);

			uint8*				Bits() const
									{ return fBits; }
			uint32				Width() const
									{ return fWidth; }
			uint32				Height() const
									{ return fHeight; }
			uint32				BytesPerRow() const
									{ return fBPR; }
			BRect				Bounds() const;

			void				CopyTo(BBitmap* bitmap, BRect area) const;

 private:
			uint8*				fBits;
			uint32				fWidth;
			uint32				fHeight;
			uint32				fBPR;
			uint32				fLeft;
			uint32				fTop;
			bool				fAdopted;
};

#endif // RENDER_BUFFER_H
