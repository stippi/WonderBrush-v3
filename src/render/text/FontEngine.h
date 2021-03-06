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

#ifndef FONT_ENGINE_H
#define FONT_ENGINE_H

#include <SupportDefs.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <agg_scanline_storage_aa.h>
#include <agg_scanline_storage_bin.h>
#include <agg_scanline_u.h>
#include <agg_scanline_bin.h>
#include <agg_path_storage_integer.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_conv_curve.h>
#include <agg_font_cache_manager.h>
#include <agg_trans_affine.h>


enum glyph_rendering {
	glyph_ren_native_mono,
	glyph_ren_native_gray8,
	glyph_ren_outline,
};


enum glyph_data_type {
	glyph_data_invalid	= 0,
	glyph_data_mono		= 1,
	glyph_data_gray8	= 2,
	glyph_data_outline	= 3
};


class FontEngine {
public:
	typedef agg::serialized_scanlines_adaptor_aa<uint8>		Gray8Adapter;
	typedef agg::serialized_scanlines_adaptor_bin			MonoAdapter;
	typedef agg::scanline_storage_aa8						ScanlineStorageAA;
	typedef agg::scanline_storage_bin						ScanlineStorageBin;
	typedef agg::serialized_integer_path_adaptor<int32, 6>	PathAdapter;

								FontEngine();
	virtual						~FontEngine();

			bool				Init(const char* fontFilePath,
									unsigned face_index, double size,
									FT_Encoding char_map,
									glyph_rendering ren_type,
									bool hinting,
									const char* font_mem = 0,
									const long font_mem_size = 0);

			int					LastError() const
									{ return fLastError; }
			unsigned			CountFaces() const;
			bool				Hinting() const
									{ return fHinting; }


			bool				PrepareGlyph(unsigned glyph_code);

			unsigned			GlyphIndex() const
									{ return fGlyphIndex; }
			unsigned			DataSize() const
									{ return fDataSize; }
			glyph_data_type		DataType() const
									{ return fDataType; }
			const agg::rect_i&	Bounds() const
									{ return fBounds; }
			double				AdvanceX() const
									{ return fAdvanceX; }
			double				AdvanceY() const
									{ return fAdvanceY; }

			void				WriteGlyphTo(uint8* data) const;

			bool				GetKerning(unsigned first, unsigned second,
									double* x, double* y);

private:
								FontEngine(const FontEngine&);
			const FontEngine&	operator=(const FontEngine&);

			int					fLastError;
			bool				fLibraryInitialized;
			FT_Library			fLibrary;	// handle to library	
			FT_Face				fFace;	  // FreeType font face handle

			glyph_rendering		fGlyphRendering;
			bool				fHinting;
		
			// members needed to generate individual glyphs according
			// to glyph rendering type
			unsigned			fGlyphIndex;
			unsigned			fDataSize;
			glyph_data_type		fDataType;
			agg::rect_i			fBounds;
			double				fAdvanceX;
			double				fAdvanceY;

			// these members are for caching memory allocations
			// when rendering glyphs		
	typedef agg::path_storage_integer<agg::int32, 6 >	PathStorageType;
	typedef agg::conv_curve<PathStorageType>			CurveConverterType;

			PathStorageType		fPath;
			CurveConverterType	fCurves;
			agg::scanline_u8	fScanlineAA;
			agg::scanline_bin	fScanlineBin;
		
			ScanlineStorageAA	fScanlineStorageAA;
			ScanlineStorageBin	fScanlineStorageBin;
};


#endif // FONT_ENGINE_H
