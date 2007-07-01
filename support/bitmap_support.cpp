/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#include "bitmap_support.h"

#include <string.h>

#include <Bitmap.h>

// demultiply_area
void
clear_area(const BBitmap* bitmap, rgb_color color, BRect area)
{
	area = area & bitmap->Bounds();

	if (!area.IsValid())
		return;

	uint8* src = (uint8*)bitmap->Bits();
	uint32 bytes = (area.IntegerWidth() + 1) * 4;
	uint32 height = area.IntegerHeight() + 1;
	uint32 bpr = bitmap->BytesPerRow();
	int32 left = (int32)area.left;
	int32 right = (int32)area.right;

	src += (int32)area.top * bpr + left * 4;


	for (uint32 y = 0; y < height; y++) {
		uint8* d = src;
		for (uint32 x = left; x <= right; x++) {
			d[0] = color.blue;
			d[1] = color.green;
			d[2] = color.blue;
			d[3] = color.alpha;
			d += 4;
		}
		src += bpr;
	}
}

// copy_area
void
copy_area(const BBitmap* source, const BBitmap* dest, BRect area)
{
	area = area & source->Bounds();
	area = area & dest->Bounds();

	if (!area.IsValid())
		return;

	uint8* src = (uint8*)source->Bits();
	uint8* dst = (uint8*)dest->Bits();
	uint32 bytes = (area.IntegerWidth() + 1) * 4;
	uint32 height = area.IntegerHeight() + 1;
	uint32 bpr = source->BytesPerRow();

	src += (int32)area.top * bpr;
	src += (int32)area.left * 4;
	dst += (int32)area.top * bpr;
	dst += (int32)area.left * 4;

	for (uint32 y = 0; y < height; y++) {
		memcpy(dst, src, bytes);
		dst += bpr;
		src += bpr;
	}
}

// blend_area
void
blend_area(const BBitmap* source, const BBitmap* dest, BRect area)
{
	area = area & source->Bounds();
	area = area & dest->Bounds();

	if (!area.IsValid())
		return;

	uint8* src = (uint8*)source->Bits();
	uint8* dst = (uint8*)dest->Bits();
	uint32 bytes = (area.IntegerWidth() + 1) * 4;
	uint32 height = area.IntegerHeight() + 1;
	uint32 bpr = source->BytesPerRow();
	int32 left = (int32)area.left;
	int32 right = (int32)area.right;

	src += (int32)area.top * bpr + left * 4;
	dst += (int32)area.top * bpr + left * 4;


	for (uint32 y = 0; y < height; y++) {
		uint8* d = dst;
		uint8* s = src;
		for (uint32 x = left; x <= right; x++) {

			uint8 alpha = 255 - s[3];
			d[0] = (uint8)((((int32)d[0] * alpha) >> 8) + s[0]);
			d[1] = (uint8)((((int32)d[1] * alpha) >> 8) + s[1]);
			d[2] = (uint8)((((int32)d[2] * alpha) >> 8) + s[2]);
			d[3] = (uint8)(255 - (((int32)alpha * (255 - d[3])) >> 8));

			d += 4;
			s += 4;
		}
		dst += bpr;
		src += bpr;
	}
}

// demultiply_area
void
demultiply_area(const BBitmap* bitmap, BRect area)
{
	area = area & bitmap->Bounds();

	if (!area.IsValid())
		return;

	uint8* src = (uint8*)bitmap->Bits();
	uint32 bytes = (area.IntegerWidth() + 1) * 4;
	uint32 height = area.IntegerHeight() + 1;
	uint32 bpr = bitmap->BytesPerRow();
	int32 left = (int32)area.left;
	int32 right = (int32)area.right;

	src += (int32)area.top * bpr + left * 4;


	for (uint32 y = 0; y < height; y++) {
		uint8* d = src;
		for (uint32 x = left; x <= right; x++) {
			if (d[3]) {
				d[0] = (uint8)((int32)d[0] * 255 / d[3]);
				d[1] = (uint8)((int32)d[1] * 255 / d[3]);
				d[2] = (uint8)((int32)d[2] * 255 / d[3]);
			}

			d += 4;
		}
		src += bpr;
	}
}
