/*
 * Copyright 2009-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef RENDER_ENGINE_H
#define RENDER_ENGINE_H

#include <agg_conv_transform.h>
#include <agg_gamma_lut.h>
#include <agg_path_storage.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_compound_aa.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_rendering_buffer.h>
#include <agg_renderer_scanline.h>
#include <agg_scanline_bin.h>
#include <agg_scanline_p.h>
#include <agg_span_allocator.h>
#include <agg_trans_perspective.h>

#include "ObjectCache.h"
#include "LayoutState.h"
#include "Scanline.h"

class BRect;
class RenderBuffer;

typedef agg::gamma_lut
			<agg::int8u, agg::int8u>		GammaTable;

typedef agg::rendering_buffer				RenderingBuffer;
typedef agg::pixfmt_bgra64_pre				PixelFormat;
typedef agg::renderer_base<PixelFormat>		BaseRenderer;
typedef agg::renderer_scanline_aa_solid<BaseRenderer>
											Renderer;

typedef agg::scanline_p8					ScanlinePacked;
typedef agg::scanline_bin					ScanlineBinary;
typedef agg::span_allocator<agg::rgba16>	SpanColorAllocator;

typedef agg::rasterizer_compound_aa
			<agg::rasterizer_sl_clip_dbl>	CompoundRasterizer;
typedef agg::rasterizer_scanline_aa
			<agg::rasterizer_sl_clip_int>	Rasterizer;
typedef agg::path_storage					PathStorage;
typedef ObjectCache<Scanline, false>		ScanlineContainer;

typedef agg::trans_perspective				Transformation;
typedef agg::conv_transform
			<PathStorage, Transformation>	TransformedPath;

// This class should become the rendering backend. Compound rasterizer
// pipeline, blending functions, etc...
// * Attachable to bitmap/surface
// * graphics state properties (only current, no stack, since layers
//   are rendered out of order)

class RenderEngine {
public:
								RenderEngine();
	virtual						~RenderEngine();

			// This method will compare the settings in the given state
			// with the current settings contained in fState, and apply
			// the differences to the engine objects. (For example, if
			// the blending mode differs, the renderer will be adjusted
			// accordingly...)
			void				Reset();
			void				SetStyle(const Style* style);

			void				AttachTo(RenderBuffer* bitmap);
			void				SetClipping(const BRect& area);

			void				SetTransformation(
									const Transformable& transformation);
			const Transformable& Transformation() const;

			// Drawing methods
			void				BlendArea(const RenderBuffer* source,
									BRect area);

			void				DrawRectangle(const BRect& rect,
									BRect area);

			void				RenderScanlines(
									const ScanlineContainer& scanlines);

private:
			void				_RenderScanlines(
									const ScanlineContainer* scanlines = NULL);

			LayoutState			fState;

			RenderingBuffer		fRenderingBuffer;
			PixelFormat			fPixelFormat;
			BaseRenderer		fBaseRenderer;

			ScanlinePacked		fScanline;
			SpanColorAllocator	fSpanAllocator;

			Rasterizer			fRasterizer;
};

#endif // RENDER_ENGINE_H
