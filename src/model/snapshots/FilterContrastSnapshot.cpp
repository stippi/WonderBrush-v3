/*
 * Copyright 2018, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#include "FilterContrastSnapshot.h"

#include <algorithm>
#include <stdio.h>

#include <Bitmap.h>

#include "rgb_hsv.h"
#include "support.h"

#include "FilterContrast.h"
#include "RenderBuffer.h"


// constructor
FilterContrastSnapshot::FilterContrastSnapshot(
		const FilterContrast* filter)
	: ObjectSnapshot(filter)
	, fOriginal(filter)
	, fContrast(filter->Contrast())
	, fCenter(filter->Center())
{
}

// destructor
FilterContrastSnapshot::~FilterContrastSnapshot()
{
}

// #pragma mark -

// Original
const Object*
FilterContrastSnapshot::Original() const
{
	return fOriginal;
}

// Sync
bool
FilterContrastSnapshot::Sync()
{
	if (ObjectSnapshot::Sync()) {
		fContrast = fOriginal->Contrast();
		fCenter = fOriginal->Center();
		return true;
	}
	return false;
}

// Render
void
FilterContrastSnapshot::Render(RenderEngine& engine,
	RenderBuffer* bitmap, BRect area) const
{
	if (fContrast == 0.0f)
		return;

	area = bitmap->Bounds() & area;

	const int top = (int)area.top;
	const int bottom = (int)area.bottom;
	const int left = (int)area.left;
	const int right = (int)area.right;

	uint8* bits = bitmap->Bits();
	bits += top * bitmap->BytesPerRow();
	bits += left * 8;

	for (int y = top; y <= bottom; y++) {
		uint16* p = (uint16*)bits;
		for (int x = left; x <= right; x++) {
			float b = p[0] / 256.0f;
			float g = p[1] / 256.0f;
			float r = p[2] / 256.0f;

			b = fCenter + (b - fCenter) * fContrast;
			g = fCenter + (g - fCenter) * fContrast;
			r = fCenter + (r - fCenter) * fContrast;

			p[0] = constrain_int32_0_65535(
				(int32)(b * 256.0f));
			p[1] = constrain_int32_0_65535(
				(int32)(g * 256.0f));
			p[2] = constrain_int32_0_65535(
				(int32)(r * 256.0f));

			// TODO: Confirm/improve this code which is supposed
			// to implement the pre-multiplication of the alpha-channel
			// (which results in the restriction that no color-channel
			// has a higher value than the alpha-channel).
			if (p[0] > p[3])
				p[0] = p[3];
			if (p[1] > p[3])
				p[1] = p[3];
			if (p[2] > p[3])
				p[2] = p[3];

			p += 4;
		}
		bits += bitmap->BytesPerRow();
	}
}
