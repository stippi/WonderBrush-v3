/*
 * Copyright 2013 Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef ALPHA_BUFFER_H
#define ALPHA_BUFFER_H

#include "PixelBuffer.h"

class AlphaBuffer;
typedef Reference<AlphaBuffer> AlphaBufferRef;

class AlphaBuffer : public PixelBuffer {
public:
								AlphaBuffer(const BRect& bounds);
								AlphaBuffer(uint32 width, uint32 height);
								AlphaBuffer(AlphaBuffer* buffer, BRect area,
									bool adopt);
								AlphaBuffer(uint8* buffer,
									uint32 width, uint32 height,
									uint32 bytesPerRow, bool adopt);

			void				Attach(uint8* buffer, uint32 width,
									uint32 height, uint32 bytesPerRow,
									bool adopt);

			AlphaBufferRef		CropUnclipped(BRect bounds) const;

protected:
	virtual PixelBuffer*		_Create(const BRect bounds) const;
};

#endif // ALPHA_BUFFER_H
