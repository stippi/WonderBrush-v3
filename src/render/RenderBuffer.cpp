/*
 * Copyright 2007,2010,2013 Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "RenderBuffer.h"

#include <new>

#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <debugger.h>

#include "RenderEngine.h"

// constructor
RenderBuffer::RenderBuffer(const BRect& bounds)
	: PixelBuffer(bounds, 8)
{
}

// constructor
RenderBuffer::RenderBuffer(uint32 width, uint32 height)
	: PixelBuffer(width, height, 8)
{
}

// constructor
RenderBuffer::RenderBuffer(const BBitmap* bitmap)
	: PixelBuffer(
		bitmap->Bounds().IntegerWidth() + 1,
		bitmap->Bounds().IntegerHeight() + 1,
		8)
{
	uint8* dst = fBits;
	uint8* src = reinterpret_cast<uint8*>(bitmap->Bits());
	uint32 srcBPR = bitmap->BytesPerRow();

	for (uint32 y = 0; y < fHeight; y++) {
		uint16* d = reinterpret_cast<uint16*>(dst);
		uint8* s = src;
		if (bitmap->ColorSpace() == B_RGBA32) {
			for (uint32 x = 0; x < fWidth; x++) {
				agg::rgba16 color(
					RenderEngine::GammaToLinear(s[2]),
					RenderEngine::GammaToLinear(s[1]),
					RenderEngine::GammaToLinear(s[0]),
					((uint16)s[3] << 8) | s[3]);
				color.premultiply();
				d[0] = color.b;
				d[1] = color.g;
				d[2] = color.r;
				d[3] = color.a;
				d += 4;
				s += 4;
			}
		} else if (bitmap->ColorSpace() == B_RGB32) {
			for (uint32 x = 0; x < fWidth; x++) {
				agg::rgba16 color(
					RenderEngine::GammaToLinear(s[2]),
					RenderEngine::GammaToLinear(s[1]),
					RenderEngine::GammaToLinear(s[0]),
					65535);
				d[0] = color.b;
				d[1] = color.g;
				d[2] = color.r;
				d[3] = color.a;
				d += 4;
				s += 4;
			}
		}
		src += srcBPR;
		dst += fBytesPerRow;
	}
}

// constructor
RenderBuffer::RenderBuffer(RenderBuffer* buffer, BRect area, bool adopt)
	: PixelBuffer(buffer, area, adopt)
{
}

// constructor
RenderBuffer::RenderBuffer(uint8* buffer, uint32 width, uint32 height,
		uint32 bytesPerRow, bool adopt)
	: PixelBuffer(buffer, width, height, 8, bytesPerRow, adopt)
{
}

// Attach
void
RenderBuffer::Attach(uint8* buffer, uint32 width, uint32 height,
	uint32 bytesPerRow, bool adopt)
{
	_Attach(buffer, width, height, 8, bytesPerRow, adopt);
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
	dst += ((int32)area.top - fTop) * fBytesPerRow;

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
		dst += fBytesPerRow;
	}
}

// CopyTo
void
RenderBuffer::CopyTo(RenderBuffer* buffer, BRect area) const
{
	PixelBuffer::CopyTo(buffer, area);
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
	src += (top - fTop) * fBytesPerRow;

	for (int32 y = 0; y < height; y++) {
		uint8* d = dst;
		uint16* s = reinterpret_cast<uint16*>(src);
		for (int32 x = left; x <= right; x++) {
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
		src += fBytesPerRow;
		dst += dstBPR;
	}
}

// CropUnclipped
RenderBufferRef
RenderBuffer::CropUnclipped(BRect bounds) const
{
	RenderBuffer* buffer = static_cast<RenderBuffer*>(_CropUnclipped(bounds));
	return RenderBufferRef(buffer, true);
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
	dst += ((int32)area.top - buffer->fTop) * dstBPR;
	uint8* src = fBits;
	src += (left - fLeft) * 8;
	src += ((int32)area.top - fTop) * fBytesPerRow;
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
		src += fBytesPerRow;
		dst += dstBPR;
	}
}

// _Create
PixelBuffer*
RenderBuffer::_Create(const BRect bounds) const
{
	return new (std::nothrow) RenderBuffer(bounds);
}
