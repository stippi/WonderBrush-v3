/*
 * Copyright 2007-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "ShapeSnapshot.h"

#include <stdio.h>

#include <agg_conv_contour.h>

#include "AutoLocker.h"
#include "Shape.h"

// constructor
ShapeSnapshot::ShapeSnapshot(const Shape* shape)
	: StyleableSnapshot(shape)
	, fOriginal(shape)
	, fPathStorage()

	, fRasterizerLock("shape lock")
	, fNeedsRasterizing(true)

	, fRasterizer()

	, fFillScanlines()
	, fStrokeScanlines()

	, fCoverAllocator()
	, fSpanAllocator()
{
	shape->GetPath(fPathStorage);
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
		fPathStorage.remove_all();
		fOriginal->GetPath(fPathStorage);

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

#if PRINT_TIMING
printf("PrepareRendering(): %lld\n", system_time() - now);
#endif

	fNeedsRasterizing = false;
}

// Render
void
ShapeSnapshot::Render(RenderEngine& engine, RenderBuffer* bitmap,
	BRect area) const
{
	PrepareRenderEngine(engine);

	engine.RenderScanlines(fFillScanlines, true);
	engine.RenderScanlines(fStrokeScanlines, false);
}

// _RasterizeShape
void
ShapeSnapshot::_RasterizeShape(Rasterizer& rasterizer, BRect bounds)
{
	_ClearScanlines();

	rasterizer.clip_box(bounds.left, bounds.top, bounds.right + 1,
		bounds.bottom + 1);

	if (FillPaint() != NULL && FillPaint()->Type() != Paint::NONE) {
		TransformedPath transformedPath(fPathStorage, LayoutedState().Matrix);
		CurvedPath curvedPath(transformedPath);
	
		rasterizer.add_path(curvedPath);
		_StoreScanlines(rasterizer, fFillScanlines);
		rasterizer.reset();
	}
	if (StrokePaint() != NULL
		&& StrokePaint()->Type() != Paint::NONE
		&& StrokeProperties() != NULL) {
		if (StrokeProperties()->StrokePosition() == CenterStroke) {
			agg::conv_curve<PathStorage> curvedPath(fPathStorage);

			agg::conv_stroke<agg::conv_curve<PathStorage> > strokedPath(
				curvedPath);
			StrokeProperties()->SetupAggConverter(strokedPath);
	
			agg::conv_transform<agg::conv_stroke<agg::conv_curve<PathStorage> >,
				Transformation>
				transformedPath(strokedPath, LayoutedState().Matrix);
	
			rasterizer.add_path(transformedPath);
		} else {
			agg::conv_curve<PathStorage> curvedPath(fPathStorage);

			agg::conv_contour<agg::conv_curve<PathStorage> > offsetPath(
				curvedPath);
			if (StrokeProperties()->StrokePosition() == InsideStroke)
				offsetPath.width(-StrokeProperties()->Width());
			else
				offsetPath.width(StrokeProperties()->Width());
			offsetPath.auto_detect_orientation(true);

			agg::conv_stroke<agg::conv_contour<agg::conv_curve<PathStorage> > >
				strokedPath(offsetPath);
			StrokeProperties()->SetupAggConverter(strokedPath);
//			strokedPath.inner_join(agg::inner_miter);
	
			agg::conv_transform<
				agg::conv_stroke<agg::conv_contour<
				agg::conv_curve<PathStorage> > >,
				Transformation>
				transformedPath(strokedPath, LayoutedState().Matrix);
	
			rasterizer.add_path(transformedPath);
		}
		_StoreScanlines(rasterizer, fStrokeScanlines);
		rasterizer.reset();
	}

	_ValidateScanlines(fFillScanlines);
	_ValidateScanlines(fStrokeScanlines);
}

// _ClearScanlines
void
ShapeSnapshot::_ClearScanlines()
{
	fCoverAllocator.Clear();
	fSpanAllocator.Clear();
	fFillScanlines.Clear();
	fStrokeScanlines.Clear();
}

// _StoreScanlines
void
ShapeSnapshot::_StoreScanlines(Rasterizer& rasterizer,
	ScanlineContainer& container)
{
	// generate scanlines
	if (!rasterizer.rewind_scanlines())
		return;

	Scanline* scanline;
	do {
		scanline = container.AppendObject();
		if (scanline == NULL)
			return;
		scanline->SetAllocators(&fCoverAllocator, &fSpanAllocator);
		scanline->reset(rasterizer.min_x(), rasterizer.max_x());
	} while (rasterizer.sweep_scanline(*scanline));

	// The last added scanline was not used anymore, so just remove it
	// again.
	container.RemoveObject();
}

// _ValidateScanlines
void
ShapeSnapshot::_ValidateScanlines(ScanlineContainer& container)
{
	// Validate the data to avoid stale pointers after relocation of
	// memory buffers.
	uint32 count = container.CountObjects();
	for (uint32 i = 0; i < count; i++) {
		Scanline* scanline = container.ObjectAtFast(i);
		scanline->Validate();
	}
}
