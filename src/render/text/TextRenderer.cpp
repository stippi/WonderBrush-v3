/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "TextRenderer.h"

#include <math.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include <ByteOrder.h>
#include <Entry.h>
#include <Message.h>
#include <Path.h>
#include <UTF8.h>

#include <agg_basics.h>
#include <agg_bounding_rect.h>
#include <agg_conv_segmentator.h>
#include <agg_trans_affine.h>

#include "FontManager.h"

#define FLIP_Y true

#define SHOW_GLYPH_BOUNDS 0

#if SHOW_GLYPH_BOUNDS
#	include <agg_conv_stroke.h>
#	include <agg_path_storage.h>
#endif

// rect_to_int
inline void
rect_to_int(BRect r,
			int32& left, int32& top, int32& right, int32& bottom)
{
	left = (int32)floorf(r.left);
	top = (int32)floorf(r.top);
	right = (int32)ceilf(r.right);
	bottom = (int32)ceilf(r.bottom);
}


#define DEFAULT_UNI_CODE_BUFFER_SIZE	2048

// #pragma mark -

// constructor
TextRenderer::TextRenderer()
	: fFontCache(FontCache::Default()),

	  fFont(),

	  fCurves(fPathAdaptor),
	  fContour(fCurves),

	  fUnicodeBuffer((char*)malloc(DEFAULT_UNI_CODE_BUFFER_SIZE)),
	  fUnicodeBufferSize(DEFAULT_UNI_CODE_BUFFER_SIZE),

	  fKerning(true),
	  fFalseBoldWidth(0.0)
{
	fCurves.approximation_scale(2.0);
	fContour.auto_detect_orientation(false);
}

// destructor
TextRenderer::~TextRenderer()
{
	free(fUnicodeBuffer);
}

// SetFont
void
TextRenderer::SetFont(const Font& font)
{
	fFont = font;
}

// SetFalseBoldWidth
void
TextRenderer::SetFalseBoldWidth(float width)
{
	fFalseBoldWidth = width * 2;
}

// StringWidth
double
TextRenderer::StringWidth(const char* utf8String, uint32 length)
{
	uint32 glyphCount;
	if (_PrepareUnicodeBuffer(utf8String, length, &glyphCount) < B_OK)
		return 0.0;

	FontCacheEntry* entry = fFontCache->FontCacheEntryFor(fFont);

	if (!entry || !entry->ReadLock()) {
		fFontCache->Recycle(entry);
		return 0.0;
	}

	bool needsWriteLock
		= !entry->HasGlyphs((uint16*)fUnicodeBuffer, glyphCount);

	if (needsWriteLock) {
		entry->ReadUnlock();
		if (!entry->WriteLock()) {
			fFontCache->Recycle(entry);
			return 0.0;
		}
	}

	// NOTE: The implementation does not take font rotation (or shear)
	// into account. Just like on R5. Should it ever be desirable to
	// "fix" this, simply use (before "return width;"):
	//
	// BPoint end(width, 0.0);
	// fEmbeddedTransformation.Transform(&end);
	// width = fabs(end.x);
	//
	// Note that shear will not have any influence on the baseline though.

	double width = 0.0;
	uint16* p = (uint16*)fUnicodeBuffer;

	double y  = 0.0;
	const GlyphCache* glyph;

	unsigned lastGlpyhCode = 0;
	for (uint32 i = 0; i < glyphCount; i++) {

		if ((glyph = entry->Glyph(*p))) {

			if (i > 0 && fKerning)
				entry->GetKerning(lastGlpyhCode, *p, &width, &y);

			width += glyph->advance_x;
		}
		lastGlpyhCode = *p;
		++p;
	}

	if (needsWriteLock)
		entry->WriteUnlock();
	else
		entry->ReadUnlock();

	fFontCache->Recycle(entry);
	return width;
}

// _PrepareUnicodeBuffer
status_t
TextRenderer::_PrepareUnicodeBuffer(const char* utf8String,
	uint32 length, uint32* glyphCount)
{
	int32 srcLength = length;
	int32 dstLength = srcLength * 4;

	// take care of adjusting buffer size
	if (dstLength > fUnicodeBufferSize) {
		fUnicodeBufferSize = dstLength;
		fUnicodeBuffer = (char*)realloc((void*)fUnicodeBuffer,
			fUnicodeBufferSize);
	}

	status_t ret;
	if (!fUnicodeBuffer) {
		ret = B_NO_MEMORY;
	} else {
		int32 state = 0;
		ret = convert_from_utf8(B_UNICODE_CONVERSION, 
								utf8String, &srcLength,
								fUnicodeBuffer, &dstLength,
								&state, B_SUBSTITUTE);
	}

	if (ret >= B_OK) {
		*glyphCount = (uint32)(dstLength / 2);
		ret = swap_data(B_INT16_TYPE, fUnicodeBuffer, dstLength,
						B_SWAP_BENDIAN_TO_HOST);
	} else {
		*glyphCount = 0;
		fprintf(stderr, "TextRenderer::_PrepareUnicodeBuffer() - "
			"UTF8 -> Unicode conversion failed: %s\n", strerror(ret));
	}

	return ret;
}

