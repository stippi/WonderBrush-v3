/*
 * Copyright 2003-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "Brush.h"

#include <stdio.h>

#include <agg_pixfmt_brush.h>
#include <agg_pixfmt_gray.h>
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
	: BaseObject()
	, fMinOpacity(0.0f)
	, fMaxOpacity(1.0f)
	, fMinRadius(0.0f)
	, fMaxRadius(1.0f)
	, fMinHardness(0.0f)
	, fMaxHardness(1.0f)
	, fFlags(FLAG_PRESSURE_CONTROLS_APHLA | FLAG_PRESSURE_CONTROLS_RADIUS
		| FLAG_TILT_CONTROLS_SHAPE)
{
}

// constructor
Brush::Brush(float minOpacity, float maxOpacity, float minRadius,
		float maxRadius, float minHardness, float maxHardness, uint32 flags)
	: BaseObject()
	, fMinOpacity(minOpacity)
	, fMaxOpacity(maxOpacity)
	, fMinRadius(minRadius)
	, fMaxRadius(maxRadius)
	, fMinHardness(minHardness)
	, fMaxHardness(maxHardness)
	, fFlags(flags)
{
}

// constructor
Brush::Brush(const Brush& other)
	: BaseObject(other)
	, fMinOpacity(other.fMinOpacity)
	, fMaxOpacity(other.fMaxOpacity)
	, fMinRadius(other.fMinRadius)
	, fMaxRadius(other.fMaxRadius)
	, fMinHardness(other.fMinHardness)
	, fMaxHardness(other.fMaxHardness)
	, fFlags(other.fFlags)
{
}

// destructor
Brush::~Brush()
{
}

// Clone
BaseObject*
Brush::Clone(CloneContext& context) const
{
	return new(std::nothrow) Brush(*this);
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

// SetMinOpacity
void
Brush::SetMinOpacity(float opacity)
{
	SetOpacity(opacity, fMaxOpacity);
}

// SetMaxOpacity
void
Brush::SetMaxOpacity(float opacity)
{
	SetOpacity(fMinOpacity, opacity);
}

// SetOpacity
void
Brush::SetOpacity(float minOpacity, float maxOpacity)
{
	if (minOpacity == fMinOpacity && maxOpacity == fMaxOpacity)
		return;

	fMinOpacity = minOpacity;
	fMaxOpacity = maxOpacity;
	Notify();
}

// SetMinRadius
void
Brush::SetMinRadius(float radius)
{
	SetRadius(radius, fMaxRadius);
}

// SetMaxRadius
void
Brush::SetMaxRadius(float radius)
{
	SetRadius(fMinRadius, radius);
}

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

// SetMinHardness
void
Brush::SetMinHardness(float hardness)
{
	SetHardness(hardness, fMaxHardness);
}

// SetMaxHardness
void
Brush::SetMaxHardness(float hardness)
{
	SetHardness(fMinHardness, hardness);
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

// SetFlags
void
Brush::SetFlags(uint32 flags, bool enable)
{
	if (enable)
		SetFlags(fFlags | flags);
	else
		SetFlags(fFlags & ~flags);
}

// SetFlags
void
Brush::SetFlags(uint32 flags)
{
	if (fFlags == flags)
		return;

	fFlags = flags;
	Notify();
}

// #pragma mark -

static inline float
value_in_range(float min, float max, float scale, bool scaled)
{
	if (scaled)
		return min + (max - min) * scale;
	else
		return max;
}


// Opacity
uint8
Brush::Opacity(float pressure) const
{
	return uint8(255.0f * value_in_range(fMinOpacity, fMaxOpacity, pressure,
		(fFlags & FLAG_PRESSURE_CONTROLS_APHLA) != 0));
}

// Radius
float
Brush::Radius(float pressure) const
{
	return value_in_range(fMinRadius, fMaxRadius, pressure,
		(fFlags & FLAG_PRESSURE_CONTROLS_RADIUS) != 0);
}

// Hardness
float
Brush::Hardness(float pressure) const
{
	return value_in_range(fMinHardness, fMaxHardness, pressure,
		(fFlags & FLAG_PRESSURE_CONTROLS_HARDNESS) != 0);
}

// #pragma mark -

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
	uint8* bits, uint32 bpr, const Transformable& transform,
	const BRect& constrainRect) const
{
//printf("Brush::Draw()\n");
//bigtime_t startTime = system_time();
	if (!constrainRect.IsValid()) {
//printf("  invalid constrain rect\n");
		return;
	}

	double radius = Radius(pressure);

	// check clipping here
	BRect clipTest(where.x - radius, where.y - radius,
		where.x + radius, where.y + radius);
	clipTest = transform.TransformBounds(clipTest);
	if (!constrainRect.Intersects(clipTest)) {
//printf("  brush shape outside clipping\n");
		return;
	}
//printf("  drawing brush (%f, %f)\n", where.x, where.y);

	double hardness = Hardness(pressure);
	uint8 opacity = Opacity(pressure);

	// Ellipse transformation
	Transformable ellipseTransform;

	// Calculate tilt deformation and rotation
	if ((fFlags & FLAG_TILT_CONTROLS_SHAPE) != 0) {
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
//bigtime_t renderTime = system_time();

	agg::rasterizer_scanline_aa<> rasterizer;
	rasterizer.clip_box(0, 0, width, height);
	rasterizer.add_path(transformedEllipse);

	agg::scanline_u8 scanlineU;

	BrushPixelFormat pixelFormat(buffer);
	pixelFormat.cover_scale(opacity);
	pixelFormat.solid((fFlags & FLAG_SOLID) != 0);

	BrushBaseRenderer rendererBase(pixelFormat);

	// special case for hardness = 1.0
	if (hardness == 1.0) {
		BrushRenderer renderer(rendererBase);
		renderer.color(Color(opacity));
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

//bigtime_t finishTime = system_time();
//printf("  init time: %lld, render time: %lld, total: %lld  (radius: %f, hardness: %f)\n",
//	   renderTime - startTime, finishTime - renderTime, finishTime - startTime, radius, hardness);
}

