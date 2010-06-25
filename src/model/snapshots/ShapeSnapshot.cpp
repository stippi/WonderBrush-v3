/*
 * Copyright 2007-2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "ShapeSnapshot.h"

#include <stdio.h>

#include <Bitmap.h>

#include "AutoLocker.h"
#include "Shape.h"

// constructor
ShapeSnapshot::ShapeSnapshot(const Shape* shape)
	: StyleableSnapshot(shape)
	, fOriginal(shape)
	, fArea(shape->Area())

	, fRasterizerLock("shape lock")
	, fNeedsRasterizing(true)

	, fRasterizer()
	, fScanlines()
	, fCoverAllocator()
	, fSpanAllocator()
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
	if (StyleableSnapshot::Sync()) {
		fArea = fOriginal->Area();
		fNeedsRasterizing = true;
		return true;
	}
	return false;
}

// Layout
void
ShapeSnapshot::Layout(LayoutContext& context, uint32 flags)
{
	Transformable previous = LayoutedState().Matrix;
	StyleableSnapshot::Layout(context, flags);
	if (previous != LayoutedState().Matrix)
		fNeedsRasterizing = true;
}

#define PRINT_TIMING 0

// PrepareRendering
void
ShapeSnapshot::PrepareRendering(BRect documentBounds)
{
	AutoLocker<BLocker> lock(fRasterizerLock);
	if (!lock.IsLocked())
		return;

	if (!fNeedsRasterizing) {
#if PRINT_TIMING
printf("PrepareRendering(): already prepared\n");
#endif
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
			scanline = fScanlines.AppendObject();
			if (scanline == NULL)
				return;
			scanline->SetAllocators(&fCoverAllocator, &fSpanAllocator);
			scanline->reset(fRasterizer.min_x(), fRasterizer.max_x());
		} while (fRasterizer.sweep_scanline(*scanline));

		// The last added scanline was not used anymore, so just remove it
		// again.
		fScanlines.RemoveObject();

		// Validate the data to avoid stale pointers after relocation of
		// memory buffers.
		uint32 count = fScanlines.CountObjects();
		for (uint32 i = 0; i < count; i++) {
			scanline = fScanlines.ObjectAtFast(i);
			scanline->Validate();
		}
	}

#if PRINT_TIMING
printf("PrepareRendering(): %lld\n", system_time() - now);
#endif

	fNeedsRasterizing = false;
}

// Render
void
ShapeSnapshot::Render(RenderEngine& engine, BBitmap* bitmap, BRect area) const
{
	engine.SetStyle(fStyle.Get());
	engine.RenderScanlines(fScanlines);
}

// _RasterizeShape
void
ShapeSnapshot::_RasterizeShape(Rasterizer& rasterizer,
	BRect bounds) const
{
	PathStorage path;
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

	TransformedPath transformedPath(path, LayoutedState().Matrix);

	rasterizer.reset();
	rasterizer.clip_box(bounds.left, bounds.top, bounds.right + 1,
		bounds.bottom + 1);

	rasterizer.add_path(transformedPath);
}

// _ClearScanlines
void
ShapeSnapshot::_ClearScanlines()
{
	fCoverAllocator.Clear();
	fSpanAllocator.Clear();
	fScanlines.Clear();
}
