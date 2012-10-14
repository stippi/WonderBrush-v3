/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <agg_conv_transform.h>
#include <agg_conv_curve.h>
#include <agg_conv_contour.h>

#include "AffineTransform.h"
#include "Font.h"
#include "FontCache.h"
#include "FontCacheEntry.h"

#include "defines.h"

class TextRenderer {
 public:
								TextRenderer();
	virtual						~TextRenderer();

	// configuring the TextRenderer
			void				SetFont(const Font& font);

			void				SetKerning(bool kerning);
			bool				Kerning() const
									{ return fKerning; }

			void				SetFalseBoldWidth(float width);
			float				FalseBoldWidth() const
									{ return fFalseBoldWidth; }

	// using the TextRenderer
			template<class FontRendererSolid, class FontRendererBin>
			BRect				RenderString(const char* utf8String,
											 uint32 length,
											 FontRendererSolid* solidRenderer,
											 FontRendererBin* binRenderer,
											 rasterizer_type* rasterizer,
											 const AffineTransform& transform,
											 const BRect& clippingFrame,
											 bool dryRun = false,
											 const escapement_delta* delta = NULL);
												 // TODO: delta is irgnored, yet

			double				StringWidth(const char* utf8String,
											uint32 length);

 private:
			void				_Update();
			status_t			_PrepareUnicodeBuffer(const char* utf8String,
													  uint32 length,
													  uint32* glyphCount);
			bool				_IsWhiteSpace(uint16 glyph) const;


	FontCache*					fFontCache;

	Font						fFont;

	// Pipeline to process the vectors glyph paths (curves + contour)
	FontCacheEntry::GlyphPathAdapter	fPathAdaptor;
	FontCacheEntry::GlyphGray8Adapter	fGray8Adaptor;
	FontCacheEntry::GlyphGray8Scanline	fGray8Scanline;
	FontCacheEntry::GlyphMonoAdapter	fMonoAdaptor;
	FontCacheEntry::GlyphMonoScanline	fMonoScanline;

	FontCacheEntry::CurveConverter		fCurves;
	FontCacheEntry::ContourConverter	fContour;

	agg::scanline_u8			fScanline;

	char*						fUnicodeBuffer;
	int32						fUnicodeBufferSize;

	bool						fKerning;
	float						fFalseBoldWidth;
};


// RenderString
inline bool
TextRenderer::_IsWhiteSpace(uint16 glyph) const
{
	// TODO: handle them all!
	if (glyph == ' ' || glyph == B_TAB)
		return true;
	return false;
}

// RenderString
template<class FontRendererSolid, class FontRendererBin>
BRect
TextRenderer::RenderString(const char* utf8String,
						   uint32 length,
						   FontRendererSolid* solidRenderer,
						   FontRendererBin* binRenderer,
						   rasterizer_type* rasterizer,
						   const AffineTransform& transform,
						   const BRect& clippingFrame,
						   bool dryRun,
						   const escapement_delta* delta)
{
//printf("RenderString(\"%s\", length: %ld, dry: %d)\n", utf8String, length, dryRun);
	// "bounds" will track the bounding box arround all glyphs that are actually drawn
	// it will be calculated in untransformed coordinates within the loop and then
	// it is transformed to the real location at the exit of the function.
	BRect bounds(LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN);

	uint32 glyphCount;
	if (_PrepareUnicodeBuffer(utf8String, length, &glyphCount) < B_OK)
		return bounds;

	bool needsOutline = true;
// TODO: Disabled because global alpha is broken with bitmap glyph caching.
// Since tickers are usually scrolled about a fractional offset anyways,
// the benefit of the bitmap cache code path is very unlikely anyways...
//	if (fFalseBoldWidth == 0.0 && transform.IsTranslationOnly()) {
//		BPoint test(B_ORIGIN);
//		transform.Transform(&test);
//		if (roundf(test.x) == test.x
//			&& roundf(test.y) == test.y) {
//			needsOutline = false;
//		}
//	}

	FontCacheEntry* entry = fFontCache->FontCacheEntryFor(fFont, needsOutline);

	if (!entry || !entry->ReadLock()) {
		fFontCache->Recycle(entry);
		return bounds;
	}

	bool needsWriteLock
		= !entry->HasGlyphs((uint16*)fUnicodeBuffer, glyphCount);

	if (needsWriteLock) {
		entry->ReadUnlock();
		if (!entry->WriteLock()) {
			fFontCache->Recycle(entry);
			return bounds;
		}
	}

	fCurves.approximation_scale(transform.scale());

	// use a transformation behind the curves
	// (only if glyph->data_type == agg::glyph_data_outline)
	// in the pipeline for the rasterizer
	typedef agg::conv_transform<FontCacheEntry::CurveConverter,
		agg::trans_affine> conv_font_trans_type;
	conv_font_trans_type transformedOutline(fCurves, transform);

	typedef agg::conv_transform<FontCacheEntry::ContourConverter,
		agg::trans_affine> conv_font_contour_trans_type;
	conv_font_contour_trans_type transformedContourOutline(fContour, transform);

	uint16* p = (uint16*)fUnicodeBuffer;

	double x  = 0.0;
	double y0 = 0.0;
	double y  = y0;

	double advanceX = 0.0;
	double advanceY = 0.0;

	// for when we bypass the transformation pipeline
	BPoint transformOffset(0.0, 0.0);
	transform.Transform(&transformOffset);

	unsigned lastGlpyhCode = 0;
	for (uint32 i = 0; i < glyphCount; i++) {

		const GlyphCache* glyph = entry->Glyph(*p);

		if (glyph) {
			if (i > 0 && fKerning)
				entry->GetKerning(lastGlpyhCode, *p, &advanceX, &advanceY);

			x += advanceX;
			y += advanceY;

			if (delta)
				x += _IsWhiteSpace(*p) ? delta->space : delta->nonspace;

			// "glyphBounds" is the bounds of the glyph transformed
			// by the x y location of the glyph along the base line,
			// it is therefor yet "untransformed".
			const agg::rect_i& r = glyph->bounds;
			BRect glyphBounds(r.x1 + x, r.y1 + y - 1, r.x2 + x + 1,
				r.y2 + y + 1);
				// NOTE: "-1"/"+ 1" converts the glyph bounding box from pixel
				// indices to pixel area coordinates

			// track bounding box
			if (glyphBounds.IsValid())
				bounds = bounds | glyphBounds;

			// render the glyph if this is not a dry run
			if (!dryRun) {
				// init the fontmanager's embedded adaptors
				// NOTE: The initialization for the "location" of
				// the glyph is different depending on wether we
				// deal with non-[rotated/sheared] text, in which
				// case we have a native (pre-rendered) FT bitmap.
				// For rotated or sheared text, we use AGG vector
				// outlines and a transformation pipeline, which
				// will be applied _after_ we retrieve the outline,
				// and that's why we simply pass x and y, which are
				// untransformed.

				// "glyphBounds" is now transformed into screen coords
				// in order to stop drawing when we are already outside
				// of the clipping frame
				if (glyph->data_type != glyph_data_outline) {
					// we cannot use the transformation pipeline
					// for vector glyphs
					double transformedX = x + transformOffset.x;
					double transformedY = y + transformOffset.y;
					entry->InitAdaptors(glyph, transformedX, transformedY,
						fMonoAdaptor, fGray8Adaptor, fPathAdaptor);
					glyphBounds.OffsetBy(transformOffset);
				} else {
					entry->InitAdaptors(glyph, x, y,
						fMonoAdaptor, fGray8Adaptor, fPathAdaptor);
					glyphBounds = transform.TransformBounds(glyphBounds);
				}

				if (clippingFrame.Intersects(glyphBounds)) {
					switch (glyph->data_type) {
						case agg::glyph_data_mono:
							agg::render_scanlines(fMonoAdaptor, 
												  fMonoScanline, 
												  *binRenderer);
							break;
		
						case agg::glyph_data_gray8:
							agg::render_scanlines(fGray8Adaptor, 
												  fGray8Scanline, 
												  *solidRenderer);
							break;
		
						case agg::glyph_data_outline: {
							rasterizer->reset();
							if (fFalseBoldWidth == 0.0)
								rasterizer->add_path(transformedOutline);
							else
								rasterizer->add_path(
									transformedContourOutline);
							agg::render_scanlines(*rasterizer, fScanline,
								*solidRenderer);
							break;
						}
						default:
							break;
					}
				}
			}

			// increment pen position
			advanceX = glyph->advance_x;
			advanceY = glyph->advance_y;
		} else {
//			// debugging
//			if (*p < 128) {
//				char c[2];
//				c[0] = (uint8)*p;
//				c[1] = 0;
//				fprintf(stderr, "failed to load glyph for '%s'\n", c);
//			} else {
//				fprintf(stderr, "failed to load glyph for %d\n", *p);
//			}
		}
		lastGlpyhCode = *p;
		++p;
	}

	if (needsWriteLock)
		entry->WriteUnlock();
	else
		entry->ReadUnlock();

	fFontCache->Recycle(entry);
	return transform.TransformBounds(bounds);
}


#endif // TEXT_RENDERER_H
