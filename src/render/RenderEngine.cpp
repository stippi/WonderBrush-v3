/*
 * Copyright 2009-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#include "RenderEngine.h"

#include <new>

#include <agg_renderer_scanline.h>
#include <agg_rounded_rect.h>

#include "RenderBuffer.h"

using std::nothrow;

// constructor
RenderEngine::RenderEngine()
	: fState()

	, fRenderingBuffer()
	, fPixelFormat(fRenderingBuffer)
	, fBaseRenderer(fPixelFormat)

	, fScanline()
	, fSpanAllocator()

	, fRasterizer()
{
}

// constructor
RenderEngine::RenderEngine(const Transformable& transformation)
	: fState()

	, fRenderingBuffer()
	, fPixelFormat(fRenderingBuffer)
	, fBaseRenderer(fPixelFormat)

	, fScanline()
	, fSpanAllocator()

	, fRasterizer()
{
	SetTransformation(transformation);
}

// destructor
RenderEngine::~RenderEngine()
{
}

// SetState
void
RenderEngine::SetStyle(const Style* style)
{
	// TODO: Check what values are different first, before assigning,
	// and apply them to the internal objects...
	if (style == NULL)
		return;

	fState.SetFillPaint(style->FillPaint());
	fState.SetStrokePaint(style->StrokePaint());

	// TODO: More stuff from style...
}

// AttachTo
void
RenderEngine::AttachTo(RenderBuffer* bitmap)
{
	if (bitmap == NULL) {
		fRenderingBuffer.attach(NULL, 0, 0, 0);
		fBaseRenderer.clip_box(0, 0, 0, 0);
		return;
	}

	// attach rendering buffer to bitmap
	fRenderingBuffer.attach((uint8*)bitmap->Bits(),
		bitmap->Width(), bitmap->Height(), bitmap->BytesPerRow());

	fBaseRenderer.clip_box(0, 0, bitmap->Width() - 1, bitmap->Height() - 1);
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
RenderEngine::BlendArea(const RenderBuffer* source, BRect area)
{
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

	uint8 globalAlpha = 255;
		// NOTE: Cover is in range 0..255 also for 16 bits/channel!

	fBaseRenderer.blend_from(sourcePixelFormat, NULL, left, top,
		globalAlpha);
}

// DrawRectangle
void
RenderEngine::DrawRectangle(const BRect& rect, BRect area)
{
	if (!fState.Matrix.TransformBounds(rect).Intersects(area))
		return;

	fRasterizer.reset();

	agg::rounded_rect roundRect(rect.left, rect.top, rect.right, rect.bottom,
		0.0);
	agg::conv_transform<agg::rounded_rect, Transformable>
		transformedRoundRect(roundRect, fState.Matrix);

	fRasterizer.add_path(transformedRoundRect);
	_RenderScanlines();
}

// RenderScanlines
void
RenderEngine::RenderScanlines(const ScanlineContainer& scanlines)
{
	_RenderScanlines(&scanlines);
}

// #pragma mark - sRGB <-> linear RGB

static const double kGamma = 2.2;
static const double kInverseGamma = 1.0 / kGamma;

static uint16 kGammaToLinear[256];
static uint8 kLinearToGamma[16384];

static bool dummy = RenderEngine::InitGammaTables();

// InitGammaTables
bool
RenderEngine::InitGammaTables()
{
#if 1
	for (uint32 i = 0; i < 256; i++)
		kGammaToLinear[i] = (uint16)(pow(i / 255.0, kGamma) * 65535.0);
	for (uint32 i = 0; i < 16384; i++)
		kLinearToGamma[i] = (uint8)(pow(i / 16383.0, kInverseGamma) * 255.0);
#else
	for (uint32 i = 0; i < 256; i++)
		kGammaToLinear[i] = i << 8 | i;
	for (uint32 i = 0; i < 16384; i++)
		kLinearToGamma[i] = i / 64;
#endif
	return true;
}

// GammaToLinear
uint16
RenderEngine::GammaToLinear(uint8 value)
{
	return kGammaToLinear[value];
}

// LinearToGamma
uint8
RenderEngine::LinearToGamma(uint16 value)
{
	// With 14 bits precision, the 8 bit values 1 and 2 are not in the
	// look up table.
	return kLinearToGamma[value >> 2];
}

// #pragma mark - hit testing

bool
RenderEngine::HitTest(const BRect& rect, const BPoint& point)
{
	fRasterizer.reset();

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

	agg::conv_transform<PathStorage, Transformable>
		transformedPath(path, fState.Matrix);

	fRasterizer.add_path(transformedPath);
	return _HitTest(point);
}

// #pragma mark -

#define PRINT_TIMING 0

// _RenderScanlines
void
RenderEngine::_RenderScanlines(const ScanlineContainer* scanlineContainer)
{
#if PRINT_TIMING
bigtime_t now = system_time();
#endif

	agg::rgba16 color(0, 0, 0, 65535);

	const Paint* paint = fState.FillPaint();
	if (paint != NULL) {
		switch (paint->Type()) {
			case Paint::COLOR:
			{
				rgb_color c = paint->Color();
				color = agg::rgba16(
					GammaToLinear(c.red),
					GammaToLinear(c.green),
					GammaToLinear(c.blue),
					c.alpha * 256 + c.alpha);
				break;
			}
			case Paint::GRADIENT:
				// TODO: ...
				break;
			case Paint::PATTERN:
				// TODO: ...
				break;

			case Paint::NONE:
			default:
				break;
		}
	}

	// TODO: Move into Paint::COLOR case above...
	color.premultiply();
	if (scanlineContainer == NULL) {
		agg::render_scanlines_aa_solid(fRasterizer, fScanline, fBaseRenderer,
			color);
	} else {
		Renderer renderer(fBaseRenderer);
		renderer.color(color);
		renderer.prepare();
#if PRINT_TIMING
int32 rendered = 0;
#endif
		int32 top = fBaseRenderer.ymin();
		int32 bottom = fBaseRenderer.ymax();

		uint32 count = scanlineContainer->CountObjects();
		for (uint32 i = 0; i < count; i++) {
			const Scanline* scanline = scanlineContainer->ObjectAtFast(i);
			int y = scanline->y();
			if (y >= top && y <= bottom) {
				renderer.render(*scanline);
#if PRINT_TIMING
rendered++;
#endif
			}
		}

#if PRINT_TIMING
printf("RenderEngine::_RenderScanlines(): %lld, %ld scanlines\n",
	system_time() - now, rendered);
#endif
	}
}

// _HitTest
bool
RenderEngine::_HitTest(const BPoint& point)
{
	return fRasterizer.hit_test(point.x, point.y);
}

