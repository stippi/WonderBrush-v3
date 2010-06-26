/*
 * Copyright 2007,2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "RenderBuffer.h"

#include <new>

#include <stdio.h>
#include <string.h>

#include <Bitmap.h>

// constructor
RenderBuffer::RenderBuffer(const BRect& bounds)
	: fBits(new(std::nothrow) uint8[(bounds.IntegerWidth() + 1) * 4
		* (bounds.IntegerHeight() + 1)])
	, fWidth(bounds.IntegerWidth() + 1)
	, fHeight(bounds.IntegerHeight() + 1)
	, fBPR(fWidth * 4)
	, fLeft(static_cast<uint32>(bounds.left))
	, fTop(static_cast<uint32>(bounds.top))
	, fAdopted(false)
{
}

// constructor
RenderBuffer::RenderBuffer(uint32 width, uint32 height)
	: fBits(new(std::nothrow) uint8[width * 4 * height])
	, fWidth(width)
	, fHeight(height)
	, fBPR(width * 4)
	, fLeft(0)
	, fTop(0)
	, fAdopted(false)
{
}

// constructor
RenderBuffer::RenderBuffer(RenderBuffer* bitmap, BRect area, bool adopt)
{
	area = area & bitmap->Bounds();

	uint8* buffer = reinterpret_cast<uint8*>(bitmap->Bits());
	uint32 width = area.IntegerWidth() + 1;
	uint32 height = area.IntegerHeight() + 1;
	uint32 bpr = bitmap->BytesPerRow();

	buffer += (int32)area.left * 4;
	buffer += (int32)area.top * bpr;

	Attach(buffer, width, height, bpr, adopt);

	fLeft = (int32)area.left;
	fTop = (int32)area.top;
}

// constructor
RenderBuffer::RenderBuffer(uint8* buffer, uint32 width, uint32 height,
		uint32 bytesPerRow, bool adopt)
	: fLeft(0)
	, fTop(0)
{
	Attach(buffer, width, height, bytesPerRow, adopt);
}

// destructor
RenderBuffer::~RenderBuffer()
{
	if (!fAdopted)
		delete[] fBits;
}

// IsValid()
bool
RenderBuffer::IsValid() const
{
	return fBits != NULL && fWidth > 0 && fHeight > 0;
}

// Attach
void
RenderBuffer::Attach(uint8* buffer, uint32 width, uint32 height,
	uint32 bytesPerRow, bool adopt)
{
	fWidth = width;
	fHeight = height;
	fAdopted = adopt;
	if (adopt) {
		fBits = buffer;
		fBPR = bytesPerRow;
	} else {
		fBPR = width * 4;
		fBits = new uint8[fBPR * height];
		uint8* dst = fBits;
		for (uint32 y = 0; y < height; y++) {
			memcpy(dst, buffer, fBPR);
			dst += fBPR;
			buffer += bytesPerRow;
		}
	}
}

// Bounds
BRect
RenderBuffer::Bounds() const
{
	BRect bounds(0, 0, fWidth - 1, fHeight - 1);
	bounds.OffsetBy(fLeft, fTop);
	return bounds;
}

// Clear
void
RenderBuffer::Clear(BRect area, const rgb_color& color)
{
	// make sure we don't clear out of bounds
	area = area & Bounds();

	uint32 left = (uint32)area.left;
	uint32 right = (uint32)area.right;
	uint32 height = area.IntegerHeight() + 1;

	uint8* dst = fBits;
	dst += (left - fLeft) * 4;
	dst += ((uint32)area.top - fTop) * fBPR;

	for (uint32 y = 0; y < height; y++) {
		uint8* d = dst;
		for (uint32 x = left; x <= right; x++) {
			d[0] = color.blue;
			d[1] = color.green;
			d[2] = color.blue;
			d[3] = color.alpha;
			d += 4;
		}
		dst += fBPR;
	}
}

// CopyTo
void
RenderBuffer::CopyTo(BBitmap* bitmap, BRect area) const
{
	// make sure we don't copy out of bounds
	area = area & bitmap->Bounds();
	area = area & Bounds();

	uint8* dst = reinterpret_cast<uint8*>(bitmap->Bits());
	uint32 dstBPR = bitmap->BytesPerRow();
	dst += (int32)area.left * 4;
	dst += (int32)area.top * dstBPR;
	uint8* src = fBits;
	src += ((int32)area.left - fLeft) * 4;
	src += ((int32)area.top - fTop) * fBPR;
	uint32 bytes = (area.IntegerWidth() + 1) * 4;
	uint32 height = area.IntegerHeight() + 1;

	for (uint32 y = 0; y < height; y++) {
		memcpy(dst, src, bytes);
		src += fBPR;
		dst += dstBPR;
	}
}

// CopyTo
void
RenderBuffer::CopyTo(RenderBuffer* buffer, BRect area) const
{
	// make sure we don't copy out of bounds
	area = area & buffer->Bounds();
	area = area & Bounds();

	uint8* dst = buffer->Bits();
	uint32 dstBPR = buffer->BytesPerRow();
	dst += (int32)area.left * 4;
	dst += (int32)area.top * dstBPR;
	uint8* src = fBits;
	src += ((int32)area.left - fLeft) * 4;
	src += ((int32)area.top - fTop) * fBPR;
	uint32 bytes = (area.IntegerWidth() + 1) * 4;
	uint32 height = area.IntegerHeight() + 1;

	for (uint32 y = 0; y < height; y++) {
		memcpy(dst, src, bytes);
		src += fBPR;
		dst += dstBPR;
	}
}

// BlendTo
void
RenderBuffer::BlendTo(RenderBuffer* buffer, BRect area) const
{
	// make sure we don't copy out of bounds
	area = area & buffer->Bounds();
	area = area & Bounds();

	uint32 left = (uint32)area.left;
	uint32 right = (uint32)area.right;

	uint8* dst = buffer->Bits();
	uint32 dstBPR = buffer->BytesPerRow();
	dst += left * 4;
	dst += (uint32)area.top * dstBPR;
	uint8* src = fBits;
	src += (left - fLeft) * 4;
	src += ((uint32)area.top - fTop) * fBPR;
	uint32 height = area.IntegerHeight() + 1;

	for (uint32 y = 0; y < height; y++) {
		uint8* d = dst;
		uint8* s = src;
		for (uint32 x = left; x <= right; x++) {

			uint8 alpha = 255 - s[3];
#if 0
			d[0] = (uint8)((((int32)d[0] * alpha) >> 8) + s[0]);
			d[1] = (uint8)((((int32)d[1] * alpha) >> 8) + s[1]);
			d[2] = (uint8)((((int32)d[2] * alpha) >> 8) + s[2]);
			d[3] = (uint8)(255 - (((int32)alpha * (255 - d[3])) >> 8));
#else
			d[0] = (uint8)((((int32)d[0] * alpha) / 255) + s[0]);
			d[1] = (uint8)((((int32)d[1] * alpha) / 255) + s[1]);
			d[2] = (uint8)((((int32)d[2] * alpha) / 255) + s[2]);
			d[3] = (uint8)(255 - (((int32)alpha * (255 - d[3])) / 255));
#endif

			d += 4;
			s += 4;
		}
		src += fBPR;
		dst += dstBPR;
	}
}
