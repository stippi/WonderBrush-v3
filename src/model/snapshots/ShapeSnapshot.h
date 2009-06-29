/*
 * Copyright 2007-2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#ifndef SHAPE_SNAPSHOT_H
#define SHAPE_SNAPSHOT_H

#include <GraphicsDefs.h>
#include <List.h>
#include <Locker.h>
#include <Rect.h>

#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_renderer_base.h>
#include <agg_rendering_buffer.h>
#include <agg_renderer_scanline.h>
#include <agg_path_storage.h>

#include "ObjectCache.h"
#include "ObjectSnapshot.h"
#include "Scanline.h"
#include "Referenceable.h"

class Shape;
class Style;

typedef agg::rendering_buffer				RenderingBuffer;
typedef agg::pixfmt_bgra32_pre				PixelFormat;
typedef agg::renderer_base<PixelFormat>		BaseRenderer;
typedef agg::renderer_scanline_aa_solid<BaseRenderer>
											Renderer;

typedef agg::rasterizer_scanline_aa<>		Rasterizer;
typedef agg::path_storage					Path;


class ShapeSnapshot : public ObjectSnapshot {
public:
								ShapeSnapshot(const Shape* shape);
	virtual						~ShapeSnapshot();

	virtual	const Object*		Original() const;
	virtual	bool				Sync();

	virtual	void				PrepareRendering(BRect documentBounds);
	virtual	void				Render(RenderEngine& engine, BBitmap* bitmap,
									BRect area) const;

private:
			void				_RasterizeShape(Rasterizer& rasterizer,
									BRect documentBounds) const;
			void				_ClearScanlines();

			const Shape*		fOriginal;
			BRect				fArea;
			Reference<Style>	fStyle;

			BLocker				fRasterizerLock;
	volatile bool				fNeedsRasterizing;

			Rasterizer			fRasterizer;

	typedef ObjectCache<Scanline, false> ScanlineContainer;
			ScanlineContainer	fScanlines2;
			CoverAllocator		fCoverAllocator;
			SpanAllocator		fSpanAllocator;
};

#endif // SHAPE_SNAPSHOT_H
