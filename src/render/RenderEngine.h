/*
 * Copyright 2009-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef RENDER_ENGINE_H
#define RENDER_ENGINE_H

#include <agg_conv_curve.h>
#include <agg_conv_stroke.h>
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

#include "BlendingMode.h"
#include "ObjectCache.h"
#include "LayoutState.h"
#include "Scanline.h"

class BRect;
class RenderBuffer;

typedef agg::gamma_lut
			<agg::int8u, agg::int8u>		GammaTable;

typedef agg::rendering_buffer				RenderingBuffer;
typedef agg::pixfmt_bgra64_pre				PixelFormat;
typedef agg::comp_op_adaptor_rgba_pre<agg::rgba16, agg::order_bgra>
											CompOpBlender;
typedef agg::pixfmt_custom_blend_rgba<CompOpBlender, RenderingBuffer>
											CompOpPixelFormat;
typedef agg::renderer_base<PixelFormat>		BaseRenderer;
typedef agg::renderer_base<CompOpPixelFormat>
											CompOpBaseRenderer;

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
typedef agg::conv_curve
			<TransformedPath>				CurvedPath;

// This class should become the rendering backend. Compound rasterizer
// pipeline, blending functions, etc...
// * Attachable to bitmap/surface
// * graphics state properties (only current, no stack, since layers
//   are rendered out of order)

class RenderEngine {
public:
								RenderEngine();
								RenderEngine(
									const Transformable& transformation);
	virtual						~RenderEngine();

			// This method will compare the settings in the given state
			// with the current settings contained in fState, and apply
			// the differences to the engine objects. (For example, if
			// the blending mode differs, the renderer will be adjusted
			// accordingly...)
			void				Reset();
			void				SetStyle(const Style& style);
			void				SetFillPaint(Paint* paint);
			void				SetStrokePaint(Paint* paint);
			void				SetStrokeProperties(
									StrokeProperties* properties);

			void				AttachTo(RenderBuffer* bitmap);
			void				SetClipping(const BRect& area);

			const RenderingBuffer& AlphaBuffer() const
									{ return fAlphaBuffer; }

			void				SetTransformation(
									const Transformable& transformation);
			const Transformable& Transformation() const;

			// Drawing methods
			void				BlendArea(const RenderBuffer* source,
									BRect area, uint8 opacity = 255,
									BlendingMode blendingMode = CompOpSrcOver);

			void				DrawRectangle(const BRect& rect,
									BRect area, double xRadius, double yRadius);
			void				DrawImage(const RenderBuffer* buffer,
									BRect area);

			void				RenderScanlines(
									const ScanlineContainer& scanlines,
									bool fillPaint);

			void				ClearAlphaBufferScanlines();
			void				RenderAlphaBufferScanlines();

	static	bool				InitGammaTables();
	static	uint16				GammaToLinear(uint8 value);
	static	uint8				LinearToGamma(uint16 value);

			bool				HitTest(const BRect& rect,
									const BPoint& point);
			bool				HitTest(PathStorage& path,
									const BPoint& point);

			status_t			Denoise(const RenderBuffer* buffer,
									const float amplitude,
									const float sharpness,
									const float anisotropy, const float alpha,
									const float sigma, const float dl,
									const float da, const float gaussPrecision,
									const unsigned int interpolationType,
									const bool fastApproximation);

private:
			// Rendering rasterizer contents or cached scanlines
			void				_RenderScanlines(bool fillPaint,
									const ScanlineContainer* scanlines = NULL);
			void				_RenderScanlines(agg::rgba16 color,
									const ScanlineContainer* scanlines = NULL);
			template<class SpanAllocator, class SpanGenerator>
			void				_RenderScanlines(SpanAllocator& spanAllocator,
									SpanGenerator& spanGenerator,
									const ScanlineContainer* scanlines = NULL);

			// Rendering contents of alpha map
			void				_RenderAlphaScanlines(bool fillPaint);
			void				_RenderAlphaScanlines(agg::rgba16 color);
			template<class SpanAllocator, class SpanGenerator>
			void				_RenderAlphaScanlines(
									SpanAllocator& spanAllocator,
									SpanGenerator& spanGenerator);

			bool				_HitTest(const BPoint& point);

			void				_ResizeAlphaBuffer();

private:
			LayoutState			fState;

			RenderingBuffer		fRenderingBuffer;

			void*				fAlphaBufferMemory;
			RenderingBuffer		fAlphaBuffer;

			PixelFormat			fPixelFormat;
			BaseRenderer		fBaseRenderer;

			CompOpPixelFormat	fCompOpPixelFormat;
			CompOpBaseRenderer	fCompOpBaseRenderer;

			ScanlinePacked		fScanline;
			SpanColorAllocator	fSpanAllocator;

			Rasterizer			fRasterizer;
};

#endif // RENDER_ENGINE_H
