/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#include "RenderBuffer.h"

#include <stdio.h>

#include <Bitmap.h>

// constructor
RenderBuffer::RenderBuffer(BBitmap* bitmap, BRect area, bool adopt)
{
	area = area & bitmap->Bounds();

	uint8* buffer = (uint8*)bitmap->Bits();
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

// CopyTo
void
RenderBuffer::CopyTo(BBitmap* bitmap, BRect area) const
{
	// make sure we don't copy out of bounds
	area = area & bitmap->Bounds();
	area = area & Bounds();

	uint32 dstBPR = bitmap->BytesPerRow();
	uint8* dst = (uint8*)bitmap->Bits();
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

