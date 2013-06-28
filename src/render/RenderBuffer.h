/*
 * Copyright 2007,2010,2013 Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef RENDER_BUFFER_H
#define RENDER_BUFFER_H

#include "PixelBuffer.h"

class BBitmap;

// The RenderBuffer is very similar to a BBitmap, with the additional features
// to represent a rectangular portion of another RenderBufffer or BBitmap that
// can either be copied or referenced. In the referenced case, the client needs
// to make sure that the source buffer remains valid for the life-time of the
// referencing RenderBuffer. Changes to the pixel data of the original buffer
// will of course be mirrored in the referencing buffer. A RenderBuffer can
// be copied into another RenderBuffer or BBitmap. A common coordinate system
// is assumed while the Bounds() property tells the layout of the source and
// target buffers. CopyTo() clips the copied buffer according to the region
// that both source and target buffers have in common with the provided area.
// It is not possible/intended to shift the buffer in the coordinate space
// during the copy process.

class RenderBuffer;
typedef Reference<RenderBuffer> RenderBufferRef;

class RenderBuffer : public PixelBuffer {
public:
								RenderBuffer(const BRect& bounds);
								RenderBuffer(uint32 width, uint32 height);
								RenderBuffer(RenderBuffer* bitmap, BRect area,
									bool adopt);
								RenderBuffer(const BBitmap* bitmap);
								RenderBuffer(uint8* buffer,
									uint32 width, uint32 height,
									uint32 bytesPerRow, bool adopt);

			void				Attach(uint8* buffer, uint32 width,
									uint32 height, uint32 bytesPerRow,
									bool adopt);

			void				Clear(BRect area, const rgb_color& color);

			void				CopyTo(RenderBuffer* buffer, BRect area) const;
			void				CopyTo(BBitmap* bitmap, BRect area) const;

			RenderBufferRef		CropUnclipped(BRect bounds) const;

			void				BlendTo(RenderBuffer* buffer, BRect area) const;

protected:
	virtual PixelBuffer*		_Create(const BRect bounds) const;
};

#endif // RENDER_BUFFER_H
