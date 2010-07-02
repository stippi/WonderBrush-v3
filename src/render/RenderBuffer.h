/*
 * Copyright 2007,2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef RENDER_BUFFER_H
#define RENDER_BUFFER_H

#include <GraphicsDefs.h>
#include <Rect.h>

#include "Referenceable.h"

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

class RenderBuffer : public Referenceable {
public:
								RenderBuffer(const BRect& bounds);
								RenderBuffer(uint32 width, uint32 height);
								RenderBuffer(RenderBuffer* bitmap, BRect area,
									bool adopt);
								RenderBuffer(const BBitmap* bitmap);
								RenderBuffer(uint8* buffer,
									uint32 width, uint32 height,
									uint32 bytesPerRow, bool adopt);
	virtual						~RenderBuffer();

			bool				IsValid() const;

			void				Attach(uint8* buffer,
									uint32 width, uint32 height,
									uint32 bytesPerRow,
									bool adopt);

	inline	uint8*				Bits() const
									{ return fBits; }
	inline	uint32				Width() const
									{ return fWidth; }
	inline	uint32				Height() const
									{ return fHeight; }
	inline	uint32				BytesPerRow() const
									{ return fBPR; }
			BRect				Bounds() const;

			void				Clear(BRect area, const rgb_color& color);

			void				CopyTo(BBitmap* bitmap, BRect area) const;
			void				CopyTo(RenderBuffer* buffer, BRect area) const;

			void				BlendTo(RenderBuffer* buffer, BRect area) const;

private:
			uint8*				fBits;
			uint32				fWidth;
			uint32				fHeight;
			uint32				fBPR;
			int32				fLeft;
			int32				fTop;
			bool				fAdopted;
};

#endif // RENDER_BUFFER_H
