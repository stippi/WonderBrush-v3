/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.4
// Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software 
// is granted provided this copyright notice appears in all copies. 
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//			mcseemagg@yahoo.com
//			http://www.antigrain.com
//----------------------------------------------------------------------------

#ifndef FONT_CACHE_ENTRY_H
#define FONT_CACHE_ENTRY_H


#include <Locker.h>

#include <agg_conv_curve.h>
#include <agg_conv_contour.h>

#include "Font.h"
#include "FontEngine.h"
#include "Referenceable.h"
#include "RWLocker.h"


struct GlyphCache {
	unsigned		glyph_index;
	uint8*			data;
	unsigned		data_size;
	glyph_data_type	data_type;
	agg::rect_i		bounds;
	double			advance_x;
	double			advance_y;
};

class FontCache;

class FontCacheEntry : public RWLocker, public Referenceable {
 public:
	typedef FontEngine::PathAdapter					GlyphPathAdapter;
	typedef FontEngine::Gray8Adapter				GlyphGray8Adapter;
	typedef GlyphGray8Adapter::embedded_scanline	GlyphGray8Scanline;
	typedef FontEngine::MonoAdapter					GlyphMonoAdapter;
	typedef GlyphMonoAdapter::embedded_scanline		GlyphMonoScanline;
	typedef agg::conv_curve<GlyphPathAdapter>		CurveConverter;
	typedef agg::conv_contour<CurveConverter>		ContourConverter;

								FontCacheEntry();
	virtual						~FontCacheEntry();

			bool				Init(const Font& font, bool forceOutline);

			bool				HasGlyphs(uint32* glyphCodes,
									size_t count) const;

			const GlyphCache*	Glyph(uint32 glyphCode);

			void				InitAdaptors(const GlyphCache* glyph,
									double x, double y,
									GlyphMonoAdapter& monoAdapter,
									GlyphGray8Adapter& gray8Adapter,
									GlyphPathAdapter& pathAdapter,
									double scale = 1.0);

			bool				GetKerning(uint32 glyphCode1,
									uint32 glyphCode2, double* x, double* y);

	static	void				GenerateSignature(char* signature,
									const Font& font, bool forceOutline);

	// private to FontCache class:
			void				UpdateUsage();
			bigtime_t			LastUsed() const
									{ return fLastUsedTime; }
			uint64				UsedCount() const
									{ return fUseCounter; }

 private:
								FontCacheEntry(const FontCacheEntry&);
			const FontCacheEntry& operator=(const FontCacheEntry&);

			class GlyphCachePool;

			GlyphCachePool*		fGlyphCache;
			FontEngine			fEngine;

	static	BLocker				sUsageUpdateLock;
			bigtime_t			fLastUsedTime;
			uint64				fUseCounter;
};

#endif // FONT_CACHE_ENTRY_H

