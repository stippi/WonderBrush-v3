/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#include "FilterSaturationSnapshot.h"

#include <algorithm>
#include <stdio.h>

#include <Bitmap.h>

#include "rgb_hsv.h"
#include "support.h"

#include "FilterSaturation.h"
#include "RenderBuffer.h"


// constructor
FilterSaturationSnapshot::FilterSaturationSnapshot(
		const FilterSaturation* filter)
	: ObjectSnapshot(filter)
	, fOriginal(filter)
	, fSaturation(filter->Saturation())
{
}

// destructor
FilterSaturationSnapshot::~FilterSaturationSnapshot()
{
}

// #pragma mark -

// Original
const Object*
FilterSaturationSnapshot::Original() const
{
	return fOriginal;
}

// Sync
bool
FilterSaturationSnapshot::Sync()
{
	if (ObjectSnapshot::Sync()) {
		fSaturation = fOriginal->Saturation();
		return true;
	}
	return false;
}

// Render
void
FilterSaturationSnapshot::Render(RenderEngine& engine,
	RenderBuffer* bitmap, BRect area) const
{
	if (fSaturation == 1.0f)
		return;

	const int top = (int)area.top;
	const int bottom = (int)area.bottom;
	const int left = (int)area.left;
	const int right = (int)area.right;

	uint8* bits = bitmap->Bits();
	bits += top * bitmap->BytesPerRow();
	bits += left * 8;

	if (fSaturation < 1.0f) {
		const int coeff = (int)(fSaturation * 256.0);
		const int oneMinusCoeff = 256 - (int)(std::min(1.0f, fSaturation) * 256.0);

		for (int y = top; y <= bottom; y++) {
			uint16* p = (uint16*)bits;
			for (int x = left; x <= right; x++) {
				int lum = 28 * p[0];	// B
				lum += 151 * p[1];		// G
				lum += 77 * p[2];		// R
				lum = lum >> 8;

				p[0] = constrain_int32_0_65535(
					(p[0] * coeff + lum * oneMinusCoeff) >> 8);
				p[1] = constrain_int32_0_65535(
					(p[1] * coeff + lum * oneMinusCoeff) >> 8);
				p[2] = constrain_int32_0_65535(
					(p[2] * coeff + lum * oneMinusCoeff) >> 8);

				p += 4;
			}
			bits += bitmap->BytesPerRow();
		}
	} else {
		for (int y = top; y <= bottom; y++) {
			uint16* p = (uint16*)bits;
			for (int x = left; x <= right; x++) {
				float b = p[0] / (256.0f * 256.0f);
				float g = p[1] / (256.0f * 256.0f);
				float r = p[2] / (256.0f * 256.0f);

				float h;
				float s;
				float v;

				RGB_to_HSV(r, g, b, h, s, v);

				s = std::min(1.0f, s * fSaturation);

				HSV_to_RGB(h, s, v, r, g, b);

				p[0] = constrain_int32_0_65535(
					(int32)(b * (256.0f * 256.0f)));
				p[1] = constrain_int32_0_65535(
					(int32)(g * (256.0f * 256.0f)));
				p[2] = constrain_int32_0_65535(
					(int32)(r * (256.0f * 256.0f)));

				p += 4;
			}
			bits += bitmap->BytesPerRow();
		}
	}
}
