/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#include "RectSnapshot.h"

#include <stdio.h>

#include <Bitmap.h>

#include "support.h"

#include "Rect.h"

// constructor
RectSnapshot::RectSnapshot(const Rect* rect)
	: ObjectSnapshot(rect)
	, fOriginal(rect)
	, fArea(rect->Area())
	, fColor(rect->Color())
{
	fColor.red = fColor.red * fColor.alpha / 255;
	fColor.green = fColor.green * fColor.alpha / 255;
	fColor.blue = fColor.blue * fColor.alpha / 255;
}

// destructor
RectSnapshot::~RectSnapshot()
{
}

// #pragma mark -

// Original
const Object*
RectSnapshot::Original() const
{
	return fOriginal;
}

// Sync
bool
RectSnapshot::Sync()
{
	if (ObjectSnapshot::Sync()) {
		fArea = fOriginal->Area();
		fColor = fOriginal->Color();
		fColor.red = fColor.red * fColor.alpha / 255;
		fColor.green = fColor.green * fColor.alpha / 255;
		fColor.blue = fColor.blue * fColor.alpha / 255;
		return true;
	}
	return false;
}

// Render
void
RectSnapshot::Render(RenderEngine& engine, BBitmap* bitmap, BRect area) const
{
	BRect rectArea = fArea;
	rectArea.left = roundf(rectArea.left);
	rectArea.top = roundf(rectArea.top);
	rectArea.right = roundf(rectArea.right);
	rectArea.bottom = roundf(rectArea.bottom);

	area = (area & rectArea) & bitmap->Bounds();
	if (!area.IsValid())
		return;

	uint32 width = area.IntegerWidth() + 1;
	uint32 height = area.IntegerHeight() + 1;
	uint32 bpr = bitmap->BytesPerRow();

	uint8* bits = (uint8*)bitmap->Bits();
	bits += (int32)area.top * bpr;
	bits += (int32)area.left * 4;

	for (uint32 y = 0; y < height; y++) {
		uint8* d = bits;
		for (uint32 x = 0; x < width; x++) {

			uint8 alpha = 255 - fColor.alpha;
			d[0] = (uint8)((((int32)d[0] * alpha) >> 8) + fColor.blue);
			d[1] = (uint8)((((int32)d[1] * alpha) >> 8) + fColor.green);
			d[2] = (uint8)((((int32)d[2] * alpha) >> 8) + fColor.red);
			d[3] = (uint8)(255 - (((int32)alpha * (255 - d[3])) >> 8));

			d += 4;
		}
		bits += bpr;
	}
}


