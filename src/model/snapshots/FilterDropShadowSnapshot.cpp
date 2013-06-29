/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "FilterDropShadowSnapshot.h"

#include <algorithm>
#include <stdio.h>

#include <Bitmap.h>

#include "AlphaBuffer.h"
#include "FilterDropShadow.h"
#include "GaussFilter.h"
#include "LayoutContext.h"
#include "RenderBuffer.h"
#include "RenderEngine.h"
#include "StackBlurFilter.h"
#include "ui_defines.h"

// constructor
FilterDropShadowSnapshot::FilterDropShadowSnapshot(
		const FilterDropShadow* filter)
	: ObjectSnapshot(filter)
	, fOriginal(filter)

	, fFilterRadius(filter->FilterRadius())
	, fOffsetX(filter->OffsetX())
	, fOffsetY(filter->OffsetY())
	, fOpacity(filter->Opacity())

	, fLayoutedFilterRadius(fFilterRadius)
	, fLayoutedOffsetX(fOffsetX)
	, fLayoutedOffsetY(fOffsetY)
{
}

// destructor
FilterDropShadowSnapshot::~FilterDropShadowSnapshot()
{
}

// #pragma mark -

// Original
const Object*
FilterDropShadowSnapshot::Original() const
{
	return fOriginal;
}

// Sync
bool
FilterDropShadowSnapshot::Sync()
{
	if (ObjectSnapshot::Sync()) {
		fFilterRadius = fOriginal->FilterRadius();
		fOffsetX = fOriginal->OffsetX();
		fOffsetY = fOriginal->OffsetY();
		fOpacity = fOriginal->Opacity();
		if (fOriginal->Color().Get() != NULL)
			fColor = fOriginal->Color()->GetColor();
		else
			fColor = kBlack;
		return true;
	}
	return false;
}

// Layout
void
FilterDropShadowSnapshot::Layout(LayoutContext& context, uint32 flags)
{
	ObjectSnapshot::Layout(context, flags);
	double scale = LayoutedState().Matrix.Scale();
	fLayoutedFilterRadius = fFilterRadius * scale;
	fLayoutedOffsetX = fOffsetX * scale;
	fLayoutedOffsetY = fOffsetY * scale;
}

// Render
void
FilterDropShadowSnapshot::Render(RenderEngine& engine, RenderBuffer* bitmap,
	BRect area) const
{
	int32 extend = (int32)ceilf(fLayoutedFilterRadius) + 3;
	BRect source(area);
	source.InsetBy(-extend, -extend);
	source.OffsetBy(-fLayoutedOffsetX, -fLayoutedOffsetY);

	AlphaBuffer alphaBuffer(source);
	
	source = source & bitmap->Bounds();

	int32 left = (int32)floorf(source.left);
	int32 top = (int32)floorf(source.top);
	int32 right = (int32)ceilf(source.right);
	int32 bottom = (int32)ceilf(source.bottom);

	// handles and byte lengths
	uint8* dst = alphaBuffer.Bits();
	uint32 dstBPR = alphaBuffer.BytesPerRow();

	// Clear alpha buffer
	memset(dst, 0, dstBPR * alphaBuffer.Height());

	uint8* src = bitmap->Bits();
	uint32 srcBPR = bitmap->BytesPerRow();

	// offsets into bitmaps
	src += left * 8 + top * srcBPR;
	dst += (left - alphaBuffer.Left()) * 2
		+ (top - alphaBuffer.Top()) * dstBPR;

	uint16 opacity = std::max((uint16)0, std::min((uint16)65535,
		(uint16)(fOpacity * 65535.0 / 255.0)));

	// first pass:
	// copy alpha channel of bitmap and blur it
	for (int32 y = top; y <= bottom; y++) {

		uint16* dstHandle = (uint16*)dst;
		uint16* srcHandle = (uint16*)src;

		for (int32 x = left; x <= right; x++) {

//			dstHandle[0] = INT_MULT(srcHandle[3], fOpacity, t);
			dstHandle[0] = (uint16)((uint32)srcHandle[3] * opacity / 65535);

			dstHandle += 1;
			srcHandle += 4;
		}
		dst += dstBPR;
		src += srcBPR;
	}

	StackBlurFilter filter;
	filter.FilterGray16(&alphaBuffer, fLayoutedFilterRadius);

	source = alphaBuffer.Bounds();
	source.InsetBy(extend, extend);
	source.OffsetBy(fLayoutedOffsetX, fLayoutedOffsetY);
	source = source & bitmap->Bounds();

	left = (int32)floorf(source.left);
	top = (int32)floorf(source.top);
	right = (int32)ceilf(source.right);
	bottom = (int32)ceilf(source.bottom);

	// reset offsets into bitmaps
	dst = bitmap->Bits();
	dstBPR = bitmap->BytesPerRow();
	src = alphaBuffer.Bits();
	srcBPR = alphaBuffer.BytesPerRow();
	dst += (left - bitmap->Left()) * 8 + (top - bitmap->Top()) * dstBPR;
	src += ((left - (int32)fLayoutedOffsetX) - alphaBuffer.Left()) * 2
		+ ((top - (int32)fLayoutedOffsetY) - alphaBuffer.Top()) * srcBPR;
	
	for (int32 y = top; y <= bottom; y++) {

		uint16* d = (uint16*)dst;
		uint16* s = (uint16*)src;

		for (int32 x = left; x <= right; x++) {
			agg::rgba16 color(
				RenderEngine::GammaToLinear(fColor.red),
				RenderEngine::GammaToLinear(fColor.green),
				RenderEngine::GammaToLinear(fColor.blue),
				s[0]);
			color.premultiply();

			uint16 alpha = 65535 - d[3];

			d[0] = (uint16)((((uint32)color.b * alpha) / 65535) + d[0]);
			d[1] = (uint16)((((uint32)color.g * alpha) / 65535) + d[1]);
			d[2] = (uint16)((((uint32)color.r * alpha) / 65535) + d[2]);
			d[3] = (uint16)(65535 - (((uint32)alpha * (65535 - s[0])) / 65535));

			d += 4;
			s += 1;
		}
		dst += dstBPR;
		src += srcBPR;
	}
}

// RebuildAreaForDirtyArea
void
FilterDropShadowSnapshot::RebuildAreaForDirtyArea(BRect& area) const
{
	// "area" is the area requested to be rendered by this
	// object.
	// This function should change the area so that
	// it includes all pixels outside the given area which
	// are required by this object to render the given area
	// correctly.
	if (fOpacity <= 0.0f)
		return;

	BRect source(area);
	source.OffsetBy(-fLayoutedOffsetX, -fLayoutedOffsetY);

	float extend = ceilf(fLayoutedFilterRadius) + 3;
		// + 3 to account for the required 3 edge pixles
	source.InsetBy(-extend, -extend);
	
	area = area | source;
}

