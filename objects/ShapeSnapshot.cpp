/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#include "ShapeSnapshot.h"

#include <stdio.h>

#include <Bitmap.h>

#include "Shape.h"

// constructor
ShapeSnapshot::ShapeSnapshot(const Shape* shape)
	: ObjectSnapshot(shape)
	, fOriginal(shape)
	, fArea(shape->Area())
	, fColor(shape->Color())

	, fRasterizerLock("shape lock")
	, fNeedsRasterizing(true)

	, fRasterizer()
	, fScanlines(256)
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
		fColor = fOriginal->Color();
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
	// TODO: reuse scanlines
	if (fRasterizer.rewind_scanlines()) {
		Scanline* scanline = new Scanline();
		scanline->reset(fRasterizer.min_x(), fRasterizer.max_x());
		while (fRasterizer.sweep_scanline(*scanline)) {
			fScanlines.AddItem(scanline);
			scanline = new Scanline();
			scanline->reset(fRasterizer.min_x(), fRasterizer.max_x());
		}
		delete scanline;
    }

#if PRINT_TIMING
printf("PrepareRendering(): %lld\n", system_time() - now);
#endif

	fNeedsRasterizing = false;
	fRasterizerLock.Unlock();
}

// Render
void
ShapeSnapshot::Render(BBitmap* bitmap, BRect area) const
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
	agg::rgba8 color(fColor.red, fColor.green,
		fColor.blue, fColor.alpha);
	color.premultiply();
	renderer.color(color);

	renderer.prepare();
	int32 count = fScanlines.CountItems();
#if PRINT_TIMING
int32 rendered = 0;
#endif
	for (int32 i = 0; i < count; i++) {
		Scanline* scanline = (Scanline*)fScanlines.ItemAtFast(i);
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
	rasterizer.clip_box(bounds.left, bounds.top, bounds.right + 1, bounds.bottom + 1);
	rasterizer.add_path(path);
}

// _ClearScanlines
void
ShapeSnapshot::_ClearScanlines()
{
	int32 count = fScanlines.CountItems();
	for (int32 i = 0; i < count; i++) {
		delete (Scanline*)fScanlines.ItemAtFast(i);
	}
	fScanlines.MakeEmpty();
}
