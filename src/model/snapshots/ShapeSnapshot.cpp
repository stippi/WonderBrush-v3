/*
 * Copyright 2007-2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "ShapeSnapshot.h"

#include <stdio.h>

#include <Bitmap.h>

#include "PaintColor.h"
	// TODO: Remove, put all the handling for Style into RenderEngine...
#include "Shape.h"
#include "Style.h"

// constructor
ShapeSnapshot::ShapeSnapshot(const Shape* shape)
	:
	ObjectSnapshot(shape),
	fOriginal(shape),
	fArea(shape->Area()),
	fStyle(shape->Style()),

	fRasterizerLock("shape lock"),
	fNeedsRasterizing(true),

	fRasterizer(),
	fScanlines2(),
	fCoverAllocator(),
	fSpanAllocator()
{
	fRasterizer.filling_rule(agg::fill_non_zero);
}

// destructor
ShapeSnapshot::~ShapeSnapshot()
{
	_ClearScanlines();
}

// #pragma mark -

// Original
const Object*
ShapeSnapshot::Original() const
{
	return fOriginal;
}

// Sync
bool
ShapeSnapshot::Sync()
{
	if (ObjectSnapshot::Sync()) {
		fArea = fOriginal->Area();
		fStyle.SetTo(fOriginal->Style());
		fNeedsRasterizing = true;
		return true;
	}
	return false;
}

#define PRINT_TIMING 0

// PrepareRendering
void
ShapeSnapshot::PrepareRendering(BRect documentBounds)
{
	if (!fRasterizerLock.Lock())
		return;

	if (!fNeedsRasterizing) {
#if PRINT_TIMING
printf("PrepareRendering(): already prepared\n");
#endif
		fRasterizerLock.Unlock();
		return;
	}

#if PRINT_TIMING
bigtime_t now = system_time();
#endif

	_RasterizeShape(fRasterizer, documentBounds);

	_ClearScanlines();

	// generate scanlines
	if (fRasterizer.rewind_scanlines()) {
		Scanline* scanline;
		do {
			scanline = fScanlines2.AppendObject();
			if (scanline == NULL)
				return;
			scanline->SetAllocators(&fCoverAllocator, &fSpanAllocator);
			scanline->reset(fRasterizer.min_x(), fRasterizer.max_x());
		} while (fRasterizer.sweep_scanline(*scanline));

		// The last added scanline was not used anymore, so just remove it
		// again.
		fScanlines2.RemoveObject();

		// Validate the data to avoid stale pointers after relocation of
		// memory buffers.
		uint32 count = fScanlines2.CountObjects();
		for (uint32 i = 0; i < count; i++) {
			scanline = fScanlines2.ObjectAtFast(i);
			scanline->Validate();
		}
	}

#if PRINT_TIMING
printf("PrepareRendering(): %lld\n", system_time() - now);
#endif

	fNeedsRasterizing = false;
	fRasterizerLock.Unlock();
}

// Render
void
ShapeSnapshot::Render(RenderEngine& engine, BBitmap* bitmap, BRect area) const
{
	area = (area & fArea) & bitmap->Bounds();
	if (!area.IsValid())
		return;

#if PRINT_TIMING
bigtime_t now = system_time();
#endif

	uint8* bits = (uint8*)bitmap->Bits();
	uint32 width = bitmap->Bounds().IntegerWidth() + 1;
	uint32 height = bitmap->Bounds().IntegerHeight() + 1;
	uint32 bpr = bitmap->BytesPerRow();
	int32 top = (int32)area.top;
	int32 bottom = (int32)area.bottom;

	RenderingBuffer buffer(bits, width, height, bpr);

	PixelFormat pixelFormat(buffer);
	BaseRenderer baseRenderer(pixelFormat);
	baseRenderer.clip_box((int32)area.left, top, (int32)area.right, bottom);

	Renderer renderer(baseRenderer);
	agg::rgba8 color(0, 0, 0, 255);
	if (fStyle.Get() != NULL) {
		Reference<Paint> paint(fStyle->FillPaint());
		if (paint.Get() != NULL) {
			switch (paint->Type()) {
				case Paint::COLOR:
					rgb_color c = dynamic_cast<PaintColor*>(
						paint.Get())->Color();
					color = agg::rgba8(c.red, c.green, c.blue, c.alpha);
					break;
				case Paint::GRADIENT:
					break;
				case Paint::PATTERN:
					break;

				case Paint::NONE:
				default:
					break;
			}
		}
	}

	color.premultiply();
	renderer.color(color);

	renderer.prepare();
#if PRINT_TIMING
int32 rendered = 0;
#endif

	uint32 count = fScanlines2.CountObjects();
	for (uint32 i = 0; i < count; i++) {
		const Scanline* scanline = fScanlines2.ObjectAtFast(i);
		int y = scanline->y();
		if (y >= top && y <= bottom) {
			renderer.render(*scanline);
#if PRINT_TIMING
rendered++;
#endif
		}
	}

#if PRINT_TIMING
printf("Render(): %lld, %ld scanlines\n", system_time() - now, rendered);
#endif
}

// _RasterizeShape
void
ShapeSnapshot::_RasterizeShape(Rasterizer& rasterizer,
	BRect bounds) const
{
	Path path;
	path.move_to(fArea.left, fArea.top);
	path.line_to((fArea.left + fArea.right) / 2,
		fArea.top + fArea.Height() / 3);

	path.line_to(fArea.right, fArea.top);
	path.line_to(fArea.right - fArea.Width() / 3,
		(fArea.top + fArea.bottom) / 2);

	path.line_to(fArea.right, fArea.bottom);
	path.line_to((fArea.left + fArea.right) / 2,
		fArea.bottom - fArea.Height() / 3);

	path.line_to(fArea.left, fArea.bottom);
	path.line_to(fArea.left + fArea.Width() / 3,
		(fArea.top + fArea.bottom) / 2);
	path.close_polygon();

	rasterizer.reset();
	rasterizer.clip_box(bounds.left, bounds.top, bounds.right + 1,
		bounds.bottom + 1);
	rasterizer.add_path(path);
}

// _ClearScanlines
void
ShapeSnapshot::_ClearScanlines()
{
	fCoverAllocator.Clear();
	fSpanAllocator.Clear();
	fScanlines2.Clear();
}
