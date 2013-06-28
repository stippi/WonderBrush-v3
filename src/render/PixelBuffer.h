/*
 * Copyright 2007,2010,2013 Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef PIXEL_BUFFER_H
#define PIXEL_BUFFER_H

#include <GraphicsDefs.h>
#include <Rect.h>

#include "Referenceable.h"

class PixelBuffer : public Referenceable {
public:
								PixelBuffer(const BRect& bounds,
									uint32 bytesPerPixel);
								PixelBuffer(uint32 width, uint32 height,
									uint32 bytesPerPixel);
								PixelBuffer(PixelBuffer* buffer, BRect area,
									bool adopt);
								PixelBuffer(uint8* buffer,
									uint32 width, uint32 height,
									uint32 bytesPerPixel,
									uint32 bytesPerRow, bool adopt);
	virtual						~PixelBuffer();

			bool				IsValid() const;

	inline	uint8*				Bits() const
									{ return fBits; }
	inline	uint32				Width() const
									{ return fWidth; }
	inline	uint32				Height() const
									{ return fHeight; }
	inline	uint32				BytesPerRow() const
									{ return fBytesPerRow; }
	inline	uint32				BytesPerPixel() const
									{ return fBytesPerPixel; }
			BRect				Bounds() const;
	inline	int32				Left() const
									{ return fLeft; }
	inline	int32				Top() const
									{ return fTop; }

			void				CopyTo(PixelBuffer* buffer, BRect area) const;

protected:
			void				_Attach(uint8* buffer,
									uint32 width, uint32 height,
									uint32 bytesPerPixel,
									uint32 bytesPerRow,
									bool adopt);

	virtual PixelBuffer*		_Create(const BRect bounds) const = 0;

			PixelBuffer*		_CropUnclipped(BRect bounds) const;

protected:
			uint8*				fBits;
			uint32				fWidth;
			uint32				fHeight;
			uint32				fBytesPerRow;
			uint32				fBytesPerPixel;
			int32				fLeft;
			int32				fTop;
			bool				fAdopted;
};

#endif // PIXEL_BUFFER_H
