/*
 * Copyright 2007-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#ifndef SHAPE_SNAPSHOT_H
#define SHAPE_SNAPSHOT_H

#include <GraphicsDefs.h>
#include <List.h>
#include <Locker.h>
#include <Rect.h>

#include "Referenceable.h"
#include "RenderEngine.h"
#include "StyleableSnapshot.h"

class Shape;


class ShapeSnapshot : public StyleableSnapshot {
public:
								ShapeSnapshot(const Shape* shape);
	virtual						~ShapeSnapshot();

	virtual	const Object*		Original() const;
	virtual	bool				Sync();

	virtual	void				Layout(LayoutContext& context, uint32 flags);

	virtual	void				PrepareRendering(BRect documentBounds);
	virtual	void				Render(RenderEngine& engine,
									RenderBuffer* bitmap, BRect area) const;

private:
			void				_RasterizeShape(Rasterizer& rasterizer,
									BRect documentBounds);
			void				_ClearScanlines();
			void				_StoreScanlines(Rasterizer& rasterizer,
									ScanlineContainer& container);
			void				_ValidateScanlines(
									ScanlineContainer& container);

private:
			const Shape*		fOriginal;
			PathStorage			fPathStorage;

			BLocker				fRasterizerLock;
	volatile bool				fNeedsRasterizing;

			Rasterizer			fRasterizer;

			ScanlineContainer	fFillScanlines;
			ScanlineContainer	fStrokeScanlines;

			CoverAllocator		fCoverAllocator;
			SpanAllocator		fSpanAllocator;
};

#endif // SHAPE_SNAPSHOT_H
