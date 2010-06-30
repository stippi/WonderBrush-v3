/*
 * Copyright 2007,2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "RenderBuffer.h"

#include <new>

#include <stdio.h>
#include <string.h>

#include <Bitmap.h>

#include "RenderEngine.h"

// constructor
RenderBuffer::RenderBuffer(const BRect& bounds)
	: fBits(new(std::nothrow) uint8[(bounds.IntegerWidth() + 1) * 8
		* (bounds.IntegerHeight() + 1)])
	, fWidth(bounds.IntegerWidth() + 1)
	, fHeight(bounds.IntegerHeight() + 1)
	, fBPR(fWidth * 8)
	, fLeft(static_cast<uint32>(bounds.left))
	, fTop(static_cast<uint32>(bounds.top))
	, fAdopted(false)
{
}

// constructor
RenderBuffer::RenderBuffer(uint32 width, uint32 height)
	: fBits(new(std::nothrow) uint8[width * 8 * height])
	, fWidth(width)
	, fHeight(height)
	, fBPR(width * 8)
	, fLeft(0)
	, fTop(0)
	, fAdopted(false)
{
}

// constructor
RenderBuffer::RenderBuffer(RenderBuffer* bitmap, BRect area, bool adopt)
{
	area = area & bitmap->Bounds();

	uint8* buffer = bitmap->Bits();
	uint32 width = area.IntegerWidth() + 1;
	uint32 height = area.IntegerHeight() + 1;
	uint32 bpr = bitmap->BytesPerRow();

	buffer += (int32)area.left * 8;
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
		fBPR = width * 8;
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

	int32 left = (int32)area.left;
	int32 right = (int32)area.right;
	int32 height = area.IntegerHeight() + 1;

	uint8* dst = fBits;
	dst += (left - fLeft) * 8;
	dst += ((uint32)area.top - fTop) * fBPR;

	agg::rgba16 linearColor(
		RenderEngine::GammaToLinear(color.red),
		RenderEngine::GammaToLinear(color.green),
		RenderEngine::GammaToLinear(color.blue),
		color.alpha * 256 + color.alpha);
	linearColor.premultiply();

	for (int32 y = 0; y < height; y++) {
		uint16* d = reinterpret_cast<uint16*>(dst);
		for (int32 x = left; x <= right; x++) {
			d[0] = linearColor.b;
			d[1] = linearColor.g;
			d[2] = linearColor.r;
			d[3] = linearColor.a;
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

	int32 left = (int32)area.left;
	int32 right = (int32)area.right;
	int32 top = (int32)area.top;
	int32 height = area.IntegerHeight() + 1;

	uint8* dst = reinterpret_cast<uint8*>(bitmap->Bits());
	uint32 dstBPR = bitmap->BytesPerRow();
	dst += (left - (int32)bitmap->Bounds().left) * 4;
	dst += (top - (int32)bitmap->Bounds().top) * dstBPR;
	uint8* src = fBits;
	src += (left - fLeft) * 8;
	src += (top - fTop) * fBPR;

	for (int32 y = 0; y < height; y++) {
		uint8* d = dst;
		uint16* s = reinterpret_cast<uint16*>(src);
		for (int32 x = 0; x <= right; x++) {
			// TODO: Right now the bitmap is solid, i.e. no transparency.
			// If there were transparency, we would have to demultiply before
			// applying inverse gamma.
			d[0] = RenderEngine::LinearToGamma(s[0]);
			d[1] = RenderEngine::LinearToGamma(s[1]);
			d[2] = RenderEngine::LinearToGamma(s[2]);
			d[3] = s[3] >> 8;
			d += 4;
			s += 4;
		}
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
	dst += ((int32)area.left - buffer->fLeft) * 8;
	dst += ((int32)area.top - buffer->fTop) * dstBPR;
	uint8* src = fBits;
	src += ((int32)area.left - fLeft) * 8;
	src += ((int32)area.top - fTop) * fBPR;
	uint32 bytes = (area.IntegerWidth() + 1) * 8;
	int32 height = area.IntegerHeight() + 1;

	for (int32 y = 0; y < height; y++) {
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

	int32 left = (int32)area.left;
	int32 right = (int32)area.right;

	uint8* dst = buffer->Bits();
	uint32 dstBPR = buffer->BytesPerRow();
	dst += (left - buffer->fLeft) * 8;
	dst += ((uint32)area.top - buffer->fTop) * dstBPR;
	uint8* src = fBits;
	src += (left - fLeft) * 8;
	src += ((uint32)area.top - fTop) * fBPR;
	int32 height = area.IntegerHeight() + 1;

	for (int32 y = 0; y < height; y++) {
		uint16* d = reinterpret_cast<uint16*>(dst);
		uint16* s = reinterpret_cast<uint16*>(src);
		for (int32 x = left; x <= right; x++) {

			uint16 alpha = 65535 - s[3];
#if 0
			d[0] = (uint16)((((uint32)d[0] * alpha) >> 16) + s[0]);
			d[1] = (uint16)((((uint32)d[1] * alpha) >> 16) + s[1]);
			d[2] = (uint16)((((uint32)d[2] * alpha) >> 16) + s[2]);
			d[3] = (uint16)(65535 - (((uint32)alpha * (65535 - d[3])) >> 16));
#else
			d[0] = (uint16)((((uint32)d[0] * alpha) / 65535) + s[0]);
			d[1] = (uint16)((((uint32)d[1] * alpha) / 65535) + s[1]);
			d[2] = (uint16)((((uint32)d[2] * alpha) / 65535) + s[2]);
			d[3] = (uint16)(65535 - (((uint32)alpha * (65535 - d[3])) / 65535));
#endif

			d += 4;
			s += 4;
		}
		src += fBPR;
		dst += dstBPR;
	}
}
