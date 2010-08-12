/*
 * Copyright 2003-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "Brush.h"

#include <stdio.h>

#include <agg_pixfmt_brush.h>
#include <agg_conv_transform.h>
#include <agg_ellipse.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_renderer_scanline.h>
#include <agg_rendering_buffer.h>
#include <agg_scanline_u.h>
#include <agg_span_allocator.h>
#include <agg_span_gradient.h>
#include <agg_span_interpolator_trans.h>

#include "support.h"

// init_gauss_table
static bool
init_gauss_table(uint8* table)
{
	for (uint32 i = 0; i < 256; i++)
		table[i] = (uint8)(255.0 * (gauss(i / 255.0)));
	return true;
}

static uint8 sGaussTable[256];
static bool dummy = init_gauss_table(sGaussTable);

// constructor
Brush::Brush()
	: fMinRadius(0.0f)
	, fMaxRadius(1.0f)
	, fMinHardness(1.0f)
	, fMaxHardness(1.0f)
{
}

// constructor
Brush::Brush(float minRadius, float maxRadius, float minHardness,
		float maxHardness)
	: fMinRadius(minRadius)
	, fMaxRadius(maxRadius)
	, fMinHardness(minHardness)
	, fMaxHardness(maxHardness)
{
}

// destructor
Brush::~Brush()
{
}

// Unarchive
status_t
Brush::Unarchive(const BMessage* archive)
{
	status_t ret = BaseObject::Unarchive(archive);

	// TODO: ...

	return ret;
}

// Archive
status_t
Brush::Archive(BMessage* into, bool deep) const
{
	status_t ret = BaseObject::Archive(into, deep);

	// TODO: ...

	return ret;
}

// AddProperties
void
Brush::AddProperties(PropertyObject* object, uint32 flags) const
{
	BaseObject::AddProperties(object, flags);

	// TODO: ...
}

// SetToPropertyObject
bool
Brush::SetToPropertyObject(const PropertyObject* object, uint32 flags)
{
	AutoNotificationSuspender _(this);

	BaseObject::SetToPropertyObject(object, flags);

	// TODO: ...

	return HasPendingNotifications();
}

// DefaultName
const char*
Brush::DefaultName() const
{
	return "Brush";
}

// #pragma mark -

// SetRadius
void
Brush::SetRadius(float minRadius, float maxRadius)
{
	if (minRadius == fMinRadius && maxRadius == fMaxRadius)
		return;

	fMinRadius = minRadius;
	fMaxRadius = maxRadius;
	Notify();
}

// Radius
float
Brush::Radius(float pressure) const
{
//	if (flags & FLAG_PRESSURE_CONTROLS_RADIUS)
		return fMinRadius + (fMaxRadius - fMinRadius) * pressure;
//	else
//		return fMaxRadius;
}

// SetHardness
void
Brush::SetHardness(float minHardness, float maxHardness)
{
	if (minHardness == fMinHardness && maxHardness == fMaxHardness)
		return;

	fMinHardness = minHardness;
	fMaxHardness = maxHardness;
	Notify();
}

// pixel format -> renderer pipeline
typedef agg::gray8										Color;
typedef agg::rendering_buffer							RenderingBuffer;
typedef agg::pixfmt_brush<Color, RenderingBuffer>		BrushPixelFormat;
typedef agg::renderer_base<BrushPixelFormat>			BrushBaseRenderer;
typedef agg::renderer_scanline_aa_solid<
	BrushBaseRenderer>									BrushRenderer;

// gradient typedefs
typedef agg::gradient_circle							GradientFunction;
typedef agg::span_interpolator_trans<Transformable>		Interpolator;

typedef agg::pod_auto_array<Color, 256>					ColorArray;
typedef agg::span_gradient<Color, Interpolator,
	GradientFunction, ColorArray>						GradientGenerator;

typedef agg::span_allocator<Color>						GradientAllocator;

typedef agg::renderer_scanline_aa<BrushBaseRenderer,
	GradientAllocator, GradientGenerator>				GradientRenderer;


// Draw
void
Brush::Draw(BPoint where, float pressure, float tiltX, float tiltY,
	float minAlpha, float maxAlpha, uint32 flags, uint8* bits, uint32 bpr,
	const Transformable& transform, const BRect& constrainRect) const
{
printf("Brush::Draw()\n");
bigtime_t startTime = system_time();
	if (!constrainRect.IsValid()) {
printf("  invalid constrain rect\n");
		return;
	}

	// radius
	double radius;
	if (flags & FLAG_PRESSURE_CONTROLS_RADIUS)
		radius = fMinRadius + (fMaxRadius - fMinRadius) * pressure;
	else
		radius = fMaxRadius;

	// check clipping here
	BRect clipTest(where.x - radius, where.y - radius,
		where.x + radius, where.y + radius);
	clipTest = transform.TransformBounds(clipTest);
	if (!constrainRect.Intersects(clipTest)) {
printf("  brush shape outside clipping\n");
		return;
	}
printf("  drawing brush (%f, %f)\n", where.x, where.y);

	// hardness
	double hardness;
	if (flags & FLAG_PRESSURE_CONTROLS_HARDNESS)
		hardness = fMinHardness + (fMaxHardness - fMinHardness) * pressure;
	else
		hardness = fMaxHardness;

	// alpha
	uint8 alpha;
	if (flags & FLAG_PRESSURE_CONTROLS_APHLA)
		alpha = uint8(255 * (minAlpha + (maxAlpha - minAlpha) * pressure));
	else
		alpha = uint8(255 * maxAlpha);

	// Ellipse transformation
	Transformable ellipseTransform;

	// Calculate tilt deformation and rotation
	if (flags & FLAG_TILT_CONTROLS_SHAPE) {
		float invTiltX = 1.0 - fabs(tiltX);
		float invTiltY = 1.0 - fabs(tiltY);
		double xScale = (sqrtf(invTiltX * invTiltX + invTiltY * invTiltY)
			/ sqrtf(2.0));
	
		double angle = calc_angle(B_ORIGIN, BPoint(tiltX, 0.0),
			BPoint(tiltX, tiltY), false);
		ellipseTransform *= agg::trans_affine_scaling(xScale, 1.0);
		ellipseTransform *= agg::trans_affine_rotation(angle);
	}

	// Calculate transformation:
	// Move ellipse to virtual brush location
	ellipseTransform *= agg::trans_affine_translation(where.x, where.y);
	// Apply global object transformation
	ellipseTransform *= transform;
	// Move ellipse to contrain window (render_buffer has no offset)
	ellipseTransform *= agg::trans_affine_translation(
		-constrainRect.left, -constrainRect.top);

	// Create transformed ellipse vertex source and rasterize it.
	agg::ellipse ellipse(0.0, 0.0, radius, radius, 64);
	agg::conv_transform<agg::ellipse, Transformable> transformedEllipse(
		ellipse, ellipseTransform);

	// Attach the AGG buffer to the memory
	RenderingBuffer buffer;
	int width = constrainRect.IntegerWidth() + 1;
	int height = constrainRect.IntegerHeight() + 1;
	bits += (int32)constrainRect.left + (int32)constrainRect.top * bpr;
	buffer.attach(bits, width, height, bpr);

	// Rasterize the ellipse
bigtime_t renderTime = system_time();

	agg::rasterizer_scanline_aa<> rasterizer;
	rasterizer.clip_box(0, 0, width, height);
	rasterizer.add_path(transformedEllipse);

	agg::scanline_u8 scanlineU;

	BrushPixelFormat pixelFormat(buffer);
	pixelFormat.cover_scale(alpha);
	pixelFormat.solid(flags & FLAG_SOLID);

	BrushBaseRenderer rendererBase(pixelFormat);

	// special case for hardness = 1.0
	if (hardness == 1.0) {
		BrushRenderer renderer(rendererBase);
		renderer.color(agg::gray8(alpha));
		agg::render_scanlines(rasterizer, scanlineU, renderer);
	} else {
		// Brush gradient transformation
		Transformable gradientTransform = ellipseTransform;
		gradientTransform.invert();
	
		// Defining the brush gradient
		GradientFunction gradientFunction;
		Interpolator interpolator(gradientTransform);
		GradientAllocator spanAllocator;
		ColorArray array(reinterpret_cast<agg::gray8*>(sGaussTable));
		GradientGenerator gradientGenerator(interpolator, gradientFunction,
			array, hardness * radius, radius * 2 - hardness * radius + 1.0);
		GradientRenderer gradientRenderer(rendererBase, spanAllocator,
			gradientGenerator);
	
		agg::render_scanlines(rasterizer, scanlineU, gradientRenderer);
	}

bigtime_t finishTime = system_time();
printf("  init time: %lld, render time: %lld, total: %lld  (radius: %f, hardness: %f)\n",
	   renderTime - startTime, finishTime - renderTime, finishTime - startTime, radius, hardness);
}

