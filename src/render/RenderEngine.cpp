/*
 * Copyright 2009-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#include "RenderEngine.h"

#include <new>

#include <agg_conv_contour.h>
#include <agg_image_accessors.h>
#include <agg_renderer_scanline.h>
#include <agg_rounded_rect.h>
#include <agg_span_gradient.h>
#include <agg_span_image_filter_rgba.h>
#include <agg_span_interpolator_linear.h>
#include <agg_span_interpolator_trans.h>
#include <agg_span_interpolator_persp.h>
#include <agg_span_subdiv_adaptor.h>

#include <CImg.h>

#include "Gradient.h"
#include "RenderBuffer.h"
#include "SetProperty.h"

using std::nothrow;

// constructor
RenderEngine::RenderEngine()
	: fState()

	, fRenderingBuffer()

	, fAlphaBufferMemory(NULL)
	, fAlphaBuffer()

	, fPixelFormat(fRenderingBuffer)
	, fBaseRenderer(fPixelFormat)

	, fCompOpPixelFormat(fRenderingBuffer)
	, fCompOpBaseRenderer(fCompOpPixelFormat)

	, fScanline()
	, fSpanAllocator()

	, fRasterizer()
{
}

// constructor
RenderEngine::RenderEngine(const Transformable& transformation)
	: fState()

	, fRenderingBuffer()

	, fAlphaBufferMemory(NULL)
	, fAlphaBuffer()

	, fPixelFormat(fRenderingBuffer)
	, fBaseRenderer(fPixelFormat)

	, fCompOpPixelFormat(fRenderingBuffer)
	, fCompOpBaseRenderer(fCompOpPixelFormat)

	, fScanline()
	, fSpanAllocator()

	, fRasterizer()
{
	SetTransformation(transformation);
}

// destructor
RenderEngine::~RenderEngine()
{
	free(fAlphaBufferMemory);
}

// SetState
void
RenderEngine::SetStyle(const Style& style)
{
	// TODO: Check what values are different first, before assigning,
	// and apply them to the internal objects...
	SetFillPaint(style.FillPaint());
	SetStrokePaint(style.StrokePaint());
	SetStrokeProperties(style.StrokeProperties());

	// TODO: More stuff from style...
}

// SetFillPaint
void
RenderEngine::SetFillPaint(Paint* paint)
{
	fState.SetFillPaint(paint);
}

// SetStrokePaint
void
RenderEngine::SetStrokePaint(Paint* paint)
{
	fState.SetStrokePaint(paint);
}

// SetStrokeProperties
void
RenderEngine::SetStrokeProperties(StrokeProperties* properties)
{
	fState.SetStrokeProperties(properties);
}

// AttachTo
void
RenderEngine::AttachTo(RenderBuffer* bitmap)
{
	if (bitmap == NULL) {
		fRenderingBuffer.attach(NULL, 0, 0, 0);
		fBaseRenderer.clip_box(0, 0, 0, 0);
		fCompOpBaseRenderer.clip_box(0, 0, 0, 0);
		return;
	}

	// attach rendering buffer to bitmap
	fRenderingBuffer.attach((uint8*)bitmap->Bits(),
		bitmap->Width(), bitmap->Height(), bitmap->BytesPerRow());

	fBaseRenderer.clip_box(0, 0, bitmap->Width() - 1, bitmap->Height() - 1);
	fCompOpBaseRenderer.clip_box(0, 0, bitmap->Width() - 1,
		bitmap->Height() - 1);

	_ResizeAlphaBuffer();
}

// SetClipping
void
RenderEngine::SetClipping(const BRect& area)
{
	BRect clipping(0, 0,
		fRenderingBuffer.width() - 1, fRenderingBuffer.height() - 1);

	clipping = area & clipping;

	fBaseRenderer.clip_box(
		(int32)clipping.left, (int32)clipping.top,
		(int32)clipping.right, (int32)clipping.bottom);
	fCompOpBaseRenderer.clip_box(
		(int32)clipping.left, (int32)clipping.top,
		(int32)clipping.right, (int32)clipping.bottom);
	fRasterizer.clip_box(
		clipping.left, clipping.top,
		clipping.right + 1, clipping.bottom + 1);
}

// SetTransformation
void
RenderEngine::SetTransformation(const Transformable& transformation)
{
	fState.Matrix = transformation;
}

// Transformation
const Transformable&
RenderEngine::Transformation() const
{
	return fState.Matrix;
}

// BlendArea
void
RenderEngine::BlendArea(const RenderBuffer* source, BRect area, uint8 opacity,
	BlendingMode blendingMode)
{
	// NOTE: Cover (opacity) is in range 0..255 also for 16 bits/channel!

	area = area & source->Bounds();

	if (!area.IsValid())
		return;

	uint8* src = (uint8*)source->Bits();
	uint32 bpr = source->BytesPerRow();
	int32 left = (int32)area.left;
	int32 top = (int32)area.top;

	src += top * bpr + left * 8;

	RenderingBuffer sourceBuffer;
	sourceBuffer.attach(src, area.IntegerWidth() + 1,
		area.IntegerHeight() + 1, bpr);

	PixelFormat sourcePixelFormat(sourceBuffer);

	switch (blendingMode) {
		case CompOpSrcOver:
			fBaseRenderer.blend_from(sourcePixelFormat, NULL, left, top,
				opacity);
			break;
		default:
			fCompOpPixelFormat.comp_op(blendingMode);
			fCompOpBaseRenderer.blend_from(sourcePixelFormat, NULL, left, top,
				opacity);
			break;
	}
}

// DrawRectangle
void
RenderEngine::DrawRectangle(const BRect& rect, BRect area,
	double xRadius, double yRadius)
{
	if (!fState.Matrix.TransformBounds(rect).Intersects(area))
		return;

	agg::rounded_rect roundRect;
	roundRect.rect(rect.left, rect.top, rect.right, rect.bottom);
	roundRect.radius(xRadius, yRadius);
	roundRect.normalize_radius();
	roundRect.approximation_scale(fState.Matrix.Scale());

	if (fState.FillPaint() != NULL
		&& fState.FillPaint()->Type() != Paint::NONE) {
		agg::conv_transform<agg::rounded_rect, Transformable>
			transformedRoundRect(roundRect, fState.Matrix);

		fRasterizer.reset();
		fRasterizer.add_path(transformedRoundRect);
		_RenderScanlines(true);
	}

	if (fState.StrokePaint() != NULL
		&& fState.StrokePaint()->Type() != Paint::NONE
		&& fState.StrokeProperties() != NULL) {
		if (fState.StrokeProperties()->StrokePosition() == CenterStroke) {
			agg::conv_stroke<agg::rounded_rect> strokedRoundRect(
				roundRect);
			fState.StrokeProperties()->SetupAggConverter(strokedRoundRect);
	
			agg::conv_transform<agg::conv_stroke<agg::rounded_rect>,
					Transformable>
				transformedRoundRect(strokedRoundRect, fState.Matrix);
	
			fRasterizer.reset();
			fRasterizer.add_path(transformedRoundRect);
		} else {
			agg::conv_contour<agg::rounded_rect> offsetPath(
				roundRect);
			if (fState.StrokeProperties()->StrokePosition() == InsideStroke)
				offsetPath.width(-fState.StrokeProperties()->Width());
			else
				offsetPath.width(fState.StrokeProperties()->Width());
			offsetPath.auto_detect_orientation(true);

			agg::conv_stroke<agg::conv_contour<agg::rounded_rect> >
				strokedPath(offsetPath);
			fState.StrokeProperties()->SetupAggConverter(strokedPath);
//			strokedPath.inner_join(agg::inner_miter);
	
			agg::conv_transform<
				agg::conv_stroke<agg::conv_contour<
				agg::rounded_rect> >,
				Transformable>
				transformedRoundRect(strokedPath, fState.Matrix);
	
			fRasterizer.reset();
			fRasterizer.add_path(transformedRoundRect);
		}
		_RenderScanlines(false);
	}
}

// DrawImage
void
RenderEngine::DrawImage(const RenderBuffer* buffer, BRect area)
{
	if (!fState.Matrix.TransformBounds(buffer->Bounds())
			.Intersects(area)) {
		return;
	}

	agg::rendering_buffer srcBuffer;
	srcBuffer.attach(buffer->Bits(), buffer->Width(), buffer->Height(),
		buffer->BytesPerRow());

	PixelFormat srcPixelFormat(srcBuffer);

	Transformable imgMatrix = fState.Matrix;
	imgMatrix.Invert();

	// path encloses image
	BRect imageRect = buffer->Bounds();
	// convert to pixel coords (versus pixel indices)
	imageRect.right++;
	imageRect.bottom++;

	fRasterizer.reset();

	agg::rounded_rect roundRect(imageRect.left, imageRect.top, imageRect.right,
		imageRect.bottom, 0.0);
	agg::conv_transform<agg::rounded_rect, Transformable>
		transformedRoundRect(roundRect, fState.Matrix);

	fRasterizer.add_path(transformedRoundRect);

//bigtime_t now = system_time();
	if (fState.Matrix.IsPerspective()) {
//printf("Perspective\n");
		typedef agg::span_interpolator_persp_exact<> Interpolator;
		Interpolator interpolator(imgMatrix);
		if (!interpolator.is_valid())
			return;
	
		typedef agg::image_accessor_clone<PixelFormat> ImageAccessor;
		ImageAccessor imageAccessor(srcPixelFormat);
	
		typedef agg::span_subdiv_adaptor<Interpolator> SubdivAdaptor;
		SubdivAdaptor subdivAdaptor(interpolator);
	
		typedef agg::span_image_resample_rgba<ImageAccessor,
			SubdivAdaptor> SpanGenerator;
		
		agg::image_filter_hanning filterKernel;
		agg::image_filter_lut filter(filterKernel, true);
	
		SpanGenerator spanGenerator(imageAccessor, subdivAdaptor, filter);
//		spanGenerator.blur(...);
	
		agg::render_scanlines_aa(fRasterizer, fScanline, fBaseRenderer,
			fSpanAllocator, spanGenerator);
	} else {
		double xScale;
		double yScale;
		fState.Matrix.GetScale(&xScale, &yScale);
		if (xScale >= 1.0 && yScale >= 1.0) {
//printf("Bilinear\n");
			typedef agg::span_interpolator_trans<Transformable> Interpolator;
			Interpolator interpolator(imgMatrix);
		
			typedef agg::span_image_filter_rgba_bilinear_clip<PixelFormat,
				Interpolator> SpanGenerator;
			SpanGenerator spanGenerator(srcPixelFormat,
				agg::rgba_pre(0, 0, 0, 0), interpolator);
		
			agg::render_scanlines_aa(fRasterizer, fScanline, fBaseRenderer,
				fSpanAllocator, spanGenerator);
		} else {
//printf("Resampling\n");
			// NOTE: This is only slightly faster (~8%) than the full blown
			// perspective case above, despite having a much simpler
			// transformer and no sub-division adapter. However, it preserves
			// a little more sharpness as well.
			typedef agg::span_interpolator_linear<agg::trans_affine>
				Interpolator;
			agg::trans_affine imgMatrixAffine(
				imgMatrix.sx,
				imgMatrix.shy,
				imgMatrix.shx,
				imgMatrix.sy,
				imgMatrix.tx,
				imgMatrix.ty);
			Interpolator interpolator(imgMatrixAffine);

			typedef agg::image_accessor_clone<PixelFormat> ImageAccessor;
			ImageAccessor imageAccessor(srcPixelFormat);

			typedef agg::span_image_resample_rgba_affine<ImageAccessor>
				SpanGenerator;

//			agg::image_filter_bilinear filterKernel;
			agg::image_filter_hanning filterKernel;
				// almost as fast as bilinear, but slightly crisper
//			agg::image_filter_blackman filterKernel(3.0);
				// 6.81 times slower than hanning, much crisper
			agg::image_filter_lut filter(filterKernel, true);

			SpanGenerator spanGenerator(imageAccessor, interpolator, filter);
//			spanGenerator.blur(...);

			agg::render_scanlines_aa(fRasterizer, fScanline, fBaseRenderer,
				fSpanAllocator, spanGenerator);
		}
	}
//printf("DrawImage(%u, %u): %lldµs\n", fRenderingBuffer.width(),
//	fRenderingBuffer.height(), system_time() - now);
}

// RenderScanlines
void
RenderEngine::RenderScanlines(const ScanlineContainer& scanlines,
	bool fillPaint)
{
	_RenderScanlines(fillPaint, &scanlines);
}

// ClearAlphaBufferScanlines
void
RenderEngine::ClearAlphaBufferScanlines()
{
	int xMin = fBaseRenderer.xmin();
	int bytes = fBaseRenderer.xmax() - xMin + 1;
	int yMin = fBaseRenderer.ymin();
	int yMax = fBaseRenderer.ymax();
	uint8* buf = fAlphaBuffer.row_ptr(yMin);
	buf += xMin;
	uint32 bpr = fAlphaBuffer.stride();
	for (int y = yMin; y <= yMax; y++) {
		memset(buf, 0, bytes);
		buf += bpr;
	}
}

// RenderAlphaBufferScanlines
void
RenderEngine::RenderAlphaBufferScanlines()
{
	_RenderAlphaScanlines(true);
}

// #pragma mark - sRGB <-> linear RGB

static const double kGamma = 2.2;
static const double kInverseGamma = 1.0 / kGamma;

static uint16 sGammaToLinear[256];
//static uint8 sLinearToGamma[16384];
static uint8 sLinearToGamma[65536];

static bool dummy = RenderEngine::InitGammaTables();

// InitGammaTables
bool
RenderEngine::InitGammaTables()
{
#if 1
//	for (uint32 i = 0; i < 16384; i++) {
//		uint16 value = pow(i / 16383.0, kInverseGamma) * 256.0;
//		sLinearToGamma[i] = value < 256 ? value : 255;
//	}
//
//	for (int32 i = 16383; i >= 0; i--) {
//		sGammaToLinear[sLinearToGamma[i]] = (i << 2) + (i >> 12);
//	}
	for (uint32 i = 0; i < 256; i++) {
		sGammaToLinear[i] = round(pow(i / 255.0, kGamma) * 65535.0);
	}
	sLinearToGamma[0] = 0;
	sLinearToGamma[1] = 1;
	for (uint32 i = 2; i < 65536; i++) {
		sLinearToGamma[i] = round(pow(i / 65535.0, kInverseGamma) * 255.0);
	}
//	for (int i = 0; i < 65536; i += 256)
//		printf("sLinearToGamma[%d] = %d\n", i, sLinearToGamma[i]);
//
//	for (int i = 0; i < 1024; i += 4)
//		printf("sLinearToGamma[%d] = %d\n", i, sLinearToGamma[i]);

#else
	for (uint32 i = 0; i < 256; i++)
		sGammaToLinear[i] = i << 8 | i;
	for (uint32 i = 0; i < 65536; i++)
		sLinearToGamma[i] = i >> 8;
#endif
	return true;
}

// GammaToLinear
uint16
RenderEngine::GammaToLinear(uint8 value)
{
	return sGammaToLinear[value];
}

// LinearToGamma
uint8
RenderEngine::LinearToGamma(uint16 value)
{
	// With 14 bits precision, the 8 bit values 1 and 2 are not in the
	// look up table.
//	return sLinearToGamma[value >> 2];
	return sLinearToGamma[value];
}

// #pragma mark - hit testing

bool
RenderEngine::HitTest(const BRect& rect, const BPoint& point)
{
	fRasterizer.reset();
	fRasterizer.clip_box(point.x, point.y, point.x + 1, point.y + 1);

	agg::rounded_rect roundRect(rect.left, rect.top, rect.right, rect.bottom,
		0.0);
	agg::conv_transform<agg::rounded_rect, Transformable>
		transformedRoundRect(roundRect, fState.Matrix);

	fRasterizer.add_path(transformedRoundRect);
	return _HitTest(point);
}

bool
RenderEngine::HitTest(PathStorage& path, const BPoint& point)
{
	fRasterizer.reset();
	fRasterizer.clip_box(point.x, point.y, point.x + 1, point.y + 1);

	agg::conv_transform<PathStorage, Transformable>
		transformedPath(path, fState.Matrix);

	fRasterizer.add_path(transformedPath);
	return _HitTest(point);
}

// #pragma mark - Filters

status_t
RenderEngine::Denoise(const RenderBuffer* buffer,
	const float amplitude, const float sharpness, const float anisotropy,
	const float alpha, const float sigma, const float dl, const float da,
	const float gaussPrecision, const unsigned int interpolationType,
	const bool fastApproximation)
{
	try {
		uint32 width = buffer->Width();
		uint32 height = buffer->Height();

		cimg_library::CImg<uint16> image(width, height, 1, 3);

		uint8* src = (uint8*)buffer->Bits();
		uint16* dst = (uint16*)image.data;
		uint32 srcBPR = buffer->BytesPerRow();

		// copy dest contents into image
		for (uint32 y = 0; y < height; y++) {
			uint16* s = (uint16*)src;
			uint16* d1 = dst;
			uint16* d2 = dst + width * height;
			uint16* d3 = dst + 2 * width * height;
			for (uint32 x = 0; x < width; x++) {
				*d1++ = s[0];
				*d2++ = s[1];
				*d3++ = s[2];
				s += 4;
			}
			src += srcBPR;
			dst += width;
		}
	
		image.blur_anisotropic(amplitude, sharpness, anisotropy,
			alpha, sigma, dl, da, gaussPrecision, interpolationType,
			fastApproximation);

		// copy result back into dest
		src = (uint8*)buffer->Bits();
		dst = (uint16*)image.data;
		for (uint32 y = 0; y < height; y++) {
			uint16* s = (uint16*)src;
			uint16* d1 = dst;
			uint16* d2 = dst + width * height;
			uint16* d3 = dst + 2 * width * height;
			for (uint32 x = 0; x < width; x++) {
				s[0] = *d1++;
				s[1] = *d2++;
				s[2] = *d3++;
				s += 4;
			}
			src += srcBPR;
			dst += width;
		}
	} catch (...) {
		printf("CImgDeNoise::ProcessBitmap() - caught exception!\n");
		return B_ERROR;
	}
	return B_OK;
}

// #pragma mark -

#define PRINT_TIMING 0

// _RenderScanlines
void
RenderEngine::_RenderScanlines(bool fillPaint,
	const ScanlineContainer* scanlineContainer)
{
	const Paint* paint = fillPaint ? fState.FillPaint() : fState.StrokePaint();
	if (paint == NULL) {
		printf("RenderEngine::_RenderScanlines() - invalid paint\n");
		return;
	}

	switch (paint->Type()) {
		case Paint::COLOR:
		{
			rgb_color c = paint->Color();
			agg::rgba16 color(
				GammaToLinear(c.red),
				GammaToLinear(c.green),
				GammaToLinear(c.blue),
				(c.alpha << 8) | c.alpha);
			color.premultiply();
			_RenderScanlines(color, fBaseRenderer, scanlineContainer);
			break;
		}
		case Paint::GRADIENT:
		{
			const agg::rgba16* gradientArray = paint->Colors();

			const GradientRef& gradient = paint->Gradient();
			Transformable transform(*gradient.Get());
			if (gradient->InheritTransformation())
				transform.Multiply(fState.Matrix);

			switch (gradient->GetType()) {
				case Gradient::CIRCULAR:
				{
					agg::gradient_radial function;
					_RenderScanlines(gradientArray, function, transform,
						scanlineContainer);
					break;
				}
				case Gradient::DIAMOND:
				{
					agg::gradient_diamond function;
					_RenderScanlines(gradientArray, function, transform,
						scanlineContainer);
					break;
				}
				case Gradient::CONIC:
				{
					agg::gradient_conic function;
					_RenderScanlines(gradientArray, function, transform,
						scanlineContainer);
					break;
				}
				case Gradient::XY:
				{
					agg::gradient_xy function;
					_RenderScanlines(gradientArray, function, transform,
						scanlineContainer);
					break;
				}
				case Gradient::SQRT_XY:
				{
					agg::gradient_sqrt_xy function;
					_RenderScanlines(gradientArray, function, transform,
						scanlineContainer);
					break;
				}
				case Gradient::LINEAR:
				default:
				{
					agg::gradient_x function;
					_RenderScanlines(gradientArray, function, transform,
						scanlineContainer);
					break;
				}
			}
			break;
		}
		case Paint::PATTERN:
		{
			// TODO: ...
			break;
		}

		case Paint::ERASE:
		{
			agg::rgba16 color(255, 255, 255, (255 << 8) | 255);
			fCompOpPixelFormat.comp_op(agg::comp_op_dst_out);
			_RenderScanlines(color, fCompOpBaseRenderer, scanlineContainer);
			break;
		}

		case Paint::NONE:
		default:
			return;
	}
}

// _RenderScanlines
template<class BaseRenderer>
void
RenderEngine::_RenderScanlines(agg::rgba16 color, BaseRenderer& baseRenderer,
	const ScanlineContainer* scanlineContainer)
{
	if (scanlineContainer == NULL) {
		// Render current contents of fRasterizer
		agg::render_scanlines_aa_solid(fRasterizer, fScanline, baseRenderer,
			color);
	} else {
		// Render cached scanlines from the container
		int32 top = baseRenderer.ymin();
		int32 bottom = baseRenderer.ymax();

		uint32 count = scanlineContainer->CountObjects();
		for (uint32 i = 0; i < count; i++) {
			const Scanline* scanline = scanlineContainer->ObjectAtFast(i);
			int y = scanline->y();
			if (y >= top && y <= bottom)
				agg::render_scanline_aa_solid(*scanline, baseRenderer, color);
		}
	}
}

// _RenderScanlines
template<class SpanAllocator, class SpanGenerator, class BaseRenderer>
void
RenderEngine::_RenderScanlines(SpanAllocator& spanAllocator,
	SpanGenerator& spanGenerator, BaseRenderer& baseRenderer,
	const ScanlineContainer* scanlineContainer)
{
	if (scanlineContainer == NULL) {
		// Render current contents of fRasterizer
		agg::render_scanlines_aa(fRasterizer, fScanline, baseRenderer,
			spanAllocator, spanGenerator);
	} else {
		// Render cached scanlines from the container
		int32 top = baseRenderer.ymin();
		int32 bottom = baseRenderer.ymax();

		uint32 count = scanlineContainer->CountObjects();
		for (uint32 i = 0; i < count; i++) {
			const Scanline* scanline = scanlineContainer->ObjectAtFast(i);
			int y = scanline->y();
			if (y >= top && y <= bottom) {
				agg::render_scanline_aa(*scanline, baseRenderer,
					spanAllocator, spanGenerator);
			}
		}
	}
}

template<class GradientFunction>
void
RenderEngine::_RenderScanlines(const agg::rgba16* gradient,
	GradientFunction function, Transformable transform,
	const ScanlineContainer* scanlines, double start, double stop)
{
	typedef agg::rgba16 ColorType;
	typedef agg::span_interpolator_trans<Transformable> InterpolatorType;
	typedef agg::pod_auto_array<ColorType, kGradientArraySize>
		ColorArrayType;
	typedef agg::span_gradient<ColorType, InterpolatorType, 
		GradientFunction, ColorArrayType> SpanGradientType;

	if (!transform.IsValid())
		return;
	
	transform.invert();

	InterpolatorType interpolator(transform);

	ColorArrayType array(gradient);
	SpanGradientType gradientGenerator(interpolator, function, array,
		start, stop);

	_RenderScanlines(fSpanAllocator, gradientGenerator, fBaseRenderer,
		scanlines);
}


// _RenderAlphaScanlines
void
RenderEngine::_RenderAlphaScanlines(bool fillPaint)
{
	const Paint* paint = fillPaint ? fState.FillPaint() : fState.StrokePaint();
	if (paint == NULL) {
		printf("RenderEngine::_RenderScanlines() - invalid paint\n");
		return;
	}

	switch (paint->Type()) {
		case Paint::COLOR:
		{
			rgb_color c = paint->Color();
			agg::rgba16 color(
				GammaToLinear(c.red),
				GammaToLinear(c.green),
				GammaToLinear(c.blue),
				(c.alpha << 8) | c.alpha);
			color.premultiply();
			_RenderAlphaScanlines(color);
			break;
		}
		case Paint::GRADIENT:
		{
			// TODO: ...
			break;
		}
		case Paint::PATTERN:
		{
			// TODO: ...
			break;
		}
		case Paint::ERASE:
		{
			// TODO: ...
			break;
		}

		case Paint::NONE:
		default:
			return;
	}
}

// _RenderAlphaScanlines
void
RenderEngine::_RenderAlphaScanlines(agg::rgba16 color)
{
	int x = fBaseRenderer.xmin();
	int length = fBaseRenderer.xmax() - x + 1;
	int yMin = fBaseRenderer.ymin();
	int yMax = fBaseRenderer.ymax();
	uint8* buf = fAlphaBuffer.row_ptr(yMin);
	buf += x;
	uint32 bpr = fAlphaBuffer.stride();
	for (int y = yMin; y <= yMax; y++) {
		fPixelFormat.blend_solid_hspan(x, y, length, color, buf);
		buf += bpr;
	}
}

// _RenderAlphaScanlines
template<class SpanAllocator, class SpanGenerator>
void
RenderEngine::_RenderAlphaScanlines(SpanAllocator& spanAllocator,
	SpanGenerator& spanGenerator)
{
	int x = fBaseRenderer.xmin();
	int length = fBaseRenderer.xmax() - x + 1;
	int yMin = fBaseRenderer.ymin();
	int yMax = fBaseRenderer.ymax();
	uint8* buf = fAlphaBuffer.row_ptr(yMin);
	buf += x;
	uint32 bpr = fAlphaBuffer.stride();
	for (int y = yMin; y <= yMax; y++) {
		typename BaseRenderer::color_type* colors
			= spanAllocator.allocate(length);
		spanGenerator.generate(colors, x, y, length);
		fBaseRenderer.blend_color_hspan(x, y, length, colors, buf, *buf);
		buf += bpr;
	}
}

// _HitTest
bool
RenderEngine::_HitTest(const BPoint& point)
{
	return fRasterizer.hit_test(point.x, point.y);
}

// _ResizeAlphaBuffer
void
RenderEngine::_ResizeAlphaBuffer()
{
	// Pixels are uint8 values
	size_t size = fRenderingBuffer.width() * fRenderingBuffer.height();
	void* newAlphaBuffer = realloc(fAlphaBufferMemory, size);
	if (newAlphaBuffer != NULL) {
		fAlphaBufferMemory = newAlphaBuffer;
		memset(fAlphaBufferMemory, 0, size);
		fAlphaBuffer.attach(static_cast<unsigned char*>(fAlphaBufferMemory),
			fRenderingBuffer.width(),
			fRenderingBuffer.height(),
			fRenderingBuffer.width());
	}
}
