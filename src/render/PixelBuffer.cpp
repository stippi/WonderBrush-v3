/*
 * Copyright 2007,2010,2013 Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "PixelBuffer.h"

#include <new>

#include <stdio.h>
#include <string.h>

#include <debugger.h>

// constructor
PixelBuffer::PixelBuffer(const BRect& bounds, uint32 bytesPerPixel)
	: fBits(new (std::nothrow) uint8[(bounds.IntegerWidth() + 1) * bytesPerPixel
		* (bounds.IntegerHeight() + 1)])
	, fWidth(bounds.IntegerWidth() + 1)
	, fHeight(bounds.IntegerHeight() + 1)
	, fBytesPerRow(fWidth * bytesPerPixel)
	, fLeft(static_cast<int32>(bounds.left))
	, fTop(static_cast<int32>(bounds.top))
	, fAdopted(false)
{
}

// constructor
PixelBuffer::PixelBuffer(uint32 width, uint32 height, uint32 bytesPerPixel)
	: fBits(new (std::nothrow) uint8[width * bytesPerPixel * height])
	, fWidth(width)
	, fHeight(height)
	, fBytesPerRow(width * bytesPerPixel)
	, fBytesPerPixel(bytesPerPixel)
	, fLeft(0)
	, fTop(0)
	, fAdopted(false)
{
}

// constructor
PixelBuffer::PixelBuffer(PixelBuffer* bitmap, BRect area, bool adopt)
	: fBits(NULL)
	, fWidth(0)
	, fHeight(0)
	, fBytesPerRow(0)
	, fBytesPerPixel(0)
	, fLeft(0)
	, fTop(0)
	, fAdopted(false)
{
	area = area & bitmap->Bounds();

	uint8* buffer = bitmap->Bits();
	uint32 width = area.IntegerWidth() + 1;
	uint32 height = area.IntegerHeight() + 1;
	uint32 bytesPerRow = bitmap->BytesPerRow();
	uint32 bytesPerPixel = bitmap->BytesPerPixel();

	buffer += (int32)area.left * bytesPerPixel;
	buffer += (int32)area.top * bytesPerRow;

	_Attach(buffer, width, height, bytesPerPixel, bytesPerRow, adopt);

	fLeft = (int32)area.left;
	fTop = (int32)area.top;
}

// constructor
PixelBuffer::PixelBuffer(uint8* buffer, uint32 width, uint32 height,
		uint32 bytesPerPixel, uint32 bytesPerRow, bool adopt)
	: fBits(NULL)
	, fWidth(0)
	, fHeight(0)
	, fBytesPerRow(0)
	, fBytesPerPixel(0)
	, fLeft(0)
	, fTop(0)
	, fAdopted(false)
{
	_Attach(buffer, width, height, bytesPerPixel, bytesPerRow, adopt);
}

// destructor
PixelBuffer::~PixelBuffer()
{
	if (!fAdopted)
		delete[] fBits;
}

// IsValid()
bool
PixelBuffer::IsValid() const
{
	return fBits != NULL && fWidth > 0 && fHeight > 0;
}

// Bounds
BRect
PixelBuffer::Bounds() const
{
	BRect bounds(0, 0, fWidth - 1, fHeight - 1);
	bounds.OffsetBy(fLeft, fTop);
	return bounds;
}

// CopyTo
void
PixelBuffer::CopyTo(PixelBuffer* buffer, BRect area) const
{
	if (buffer->BytesPerPixel() != fBytesPerPixel)
		debugger("Can't CopyTo() buffer with different bytes per pixel!");

	// make sure we don't copy out of bounds
	area = area & buffer->Bounds();
	area = area & Bounds();

	uint8* dst = buffer->Bits();
	uint32 dstBPR = buffer->BytesPerRow();
	dst += ((int32)area.left - buffer->fLeft) * fBytesPerPixel;
	dst += ((int32)area.top - buffer->fTop) * dstBPR;
	uint8* src = fBits;
	src += ((int32)area.left - fLeft) * fBytesPerPixel;
	src += ((int32)area.top - fTop) * fBytesPerRow;
	int32 bytes = (area.IntegerWidth() + 1) * fBytesPerPixel;
	int32 height = area.IntegerHeight() + 1;

	for (int32 y = 0; y < height; y++) {
		memcpy(dst, src, bytes);
		src += fBytesPerRow;
		dst += dstBPR;
	}
}

// #pragma mark -

// _Attach
void
PixelBuffer::_Attach(uint8* buffer, uint32 width, uint32 height,
	uint32 bytesPerPixel, uint32 bytesPerRow, bool adopt)
{
	if (!fAdopted)
		delete[] fBits;

	fWidth = width;
	fHeight = height;
	fAdopted = adopt;
	fBytesPerPixel = bytesPerPixel;
	if (adopt) {
		fBits = buffer;
		fBytesPerRow = bytesPerRow;
		if (fBytesPerRow < width * fBytesPerPixel)
			debugger("Buffer size insufficient for given width.");
	} else {
		fBytesPerRow = width * fBytesPerPixel;
		fBits = new uint8[fBytesPerRow * height];
		uint8* dst = fBits;
		for (uint32 y = 0; y < height; y++) {
			memcpy(dst, buffer, fBytesPerRow);
			dst += fBytesPerRow;
			buffer += bytesPerRow;
		}
	}
}


