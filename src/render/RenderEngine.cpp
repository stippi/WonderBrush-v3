/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#include "RenderEngine.h"

#include <new>

#include <Bitmap.h>

#include <agg_renderer_scanline.h>
#include <agg_rounded_rect.h>

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
RenderEngine::AttachTo(BBitmap* bitmap)
{
	if (bitmap == NULL) {
		fRenderingBuffer.attach(NULL, 0, 0, 0);
		fBaseRenderer.clip_box(0, 0, 0, 0);
		return;
	}

	// attach rendering buffer to bitmap
	fRenderingBuffer.attach((uint8*)bitmap->Bits(),
		bitmap->Bounds().IntegerWidth() + 1,
		bitmap->Bounds().IntegerHeight() + 1, bitmap->BytesPerRow());

	fBaseRenderer.clip_box(0, 0, bitmap->Bounds().IntegerWidth(),
		bitmap->Bounds().IntegerHeight());
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

// BlendArea
void
RenderEngine::BlendArea(const BBitmap* source, BRect area)
{
	area = area & source->Bounds();

	if (!area.IsValid())
		return;

	uint8* src = (uint8*)source->Bits();
	uint32 bpr = source->BytesPerRow();
	int32 left = (int32)area.left;
	int32 top = (int32)area.top;

	src += top * bpr + left * 4;

	RenderingBuffer sourceBuffer;
	sourceBuffer.attach(src, area.IntegerWidth() + 1,
		area.IntegerHeight() + 1, bpr);

	PixelFormat sourcePixelFormat(sourceBuffer);

	uint8 globalAlpha = 255;

	fBaseRenderer.blend_from(sourcePixelFormat, NULL, left, top,
		globalAlpha);
}

// DrawRectangle
void
RenderEngine::DrawRectangle(const BRect& rect, BRect area)
{
	if (!area.Intersects(rect))
		return;

	agg::rounded_rect roundRect(rect.left, rect.top, rect.right, rect.bottom,
		0.0);

	fRasterizer.reset();
	fRasterizer.add_path(roundRect);

	_RenderScanlines();
}

// RenderScanlines
void
RenderEngine::RenderScanlines(const ScanlineContainer& scanlines)
{
	_RenderScanlines(&scanlines);
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

	agg::rgba8 color(0, 0, 0, 255);

	const Paint* paint = fState.FillPaint();
	if (paint != NULL) {
		switch (paint->Type()) {
			case Paint::COLOR:
			{
				rgb_color c = paint->Color();
				color = agg::rgba8(c.red, c.green, c.blue, c.alpha);
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
