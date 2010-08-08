/*
 * Copyright 2003-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */

#include "Brush.h"

#include <stdio.h>

#include <agg_conv_transform.h>
#include <agg_ellipse.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_renderer_scanline.h>
#include <agg_rendering_buffer.h>
#include <agg_scanline_u.h>
#include <agg_span_gradient.h>
#include <agg_span_interpolator_trans.h>

#include "support.h"

// init_gauss_table
static bool
init_gauss_table(const uint8* table)
{
	for (uint32 i = 0; i < 256; i++)
		table[i] = (uint8)(255.0 * (gauss(i / 255.0)));
	return true;
}

const uint8 Brush::sGaussTable[256];
static bool dummy = init_gauss_table(sGaussTable);

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

// SetRadius
void
SetRadius(float minRadius, float maxRadius)
{
	if (minRadius == fMinRadius && maxRadius == fMaxRadius)
		return;

	fMinRadius = minRadius;
	fMaxRadius = maxRadius;
	Notify();
}

// SetHardness
void
SetRadius(float minHardness, float maxHardness)
{
	if (minHardness == fMinHardness && maxHardness == fMaxHardness)
		return;

	fMinHardness = minHardness;
	fMaxHardness = maxHardness;
	Notify();
}

typedef agg::renderer_base<agg::pixfmt_brush8>					renderer_base;
//typedef agg::renderer_scanline_u_solid<renderer_base>			renderer_type;
typedef agg::renderer_scanline_aa_solid<renderer_base>			renderer_type;
typedef agg::gradient_circle									gradient_function;
typedef agg::span_interpolator_trans<Transformable>				interpolator_type;

typedef agg::pod_auto_array<agg::gray8, 256>					color_array_type;
typedef agg::span_gradient<agg::gray8,
						   interpolator_type,
						   gradient_function,
						   color_array_type>					gradient_generator;

typedef agg::span_allocator<agg::gray8>							gradient_allocator;

typedef agg::renderer_scanline_aa<renderer_base,
								  gradient_generator>			gradient_renderer;
//typedef agg::renderer_scanline_u<renderer_base,
//								  gradient_generator>			gradient_renderer;



// Draw
void
Brush::Draw(BPoint where, float pressure, float tiltX, float tiltY,
	float minAlpha, float maxAlphaa, uint32 flags, uint8* bits, uint32 bpr,
	const Transformable& transform, const BRect& constrainRect) const
{
//printf("Brush::Draw()\n");
//bigtime_t startTime = system_time();
	if (!constrainRect.IsValid()) {
//printf("invalid constrain rect\n");
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
//printf("brush shape outside clipping\n");
		return;
	}
//printf("drawing brush (%f, %f)\n", where.x, where.y);

	// hardness
	double hardness;
	if (flags & FLAG_PRESSURE_CONTROLS_HARDNESS)
		hardness = fMinHardness + (fMaxHardness - fMinHardness) * pressure;
	else
		hardness = fMaxHardness;


	agg::rasterizer_scanline_aa<> rasterizer;

	// attach the AGG buffer to the bitmap
	agg::rendering_buffer buffer;
	int width = constrainRect.IntegerWidth() + 1;
	int height = constrainRect.IntegerHeight() + 1;
	bits += (int32)constrainRect.left + (int32)constrainRect.top * bpr;
	buffer.attach(bits, width, height, bpr);

	rasterizer.clip_box(0, 0, width, height);

	agg::pixfmt_brush8 pixelFormat(buffer);

	renderer_base rendererBase(pixelFormat);
	renderer_type renderer(rendererBase);
	agg::scanline_u8 scanlineU;

	uint8 alpha;
	if (flags & FLAG_PRESSURE_CONTROLS_APHLA)
		alpha = uint8(255 * (minAlpha + (maxAlpha - minAlpha) * pressure));
	else
		alpha = uint8(255 * maxAlpha);

	renderer.color(agg::gray8(alpha));

	// Ellipse transformation
	Transformable ellipseTransform;

	// Calculate tilt deformation and rotation
	if (flags & FLAG_TILT_CONTROLS_SHAPE) {
		float invTiltX = 1.0 - fabs(tiltX);
		float invTiltY = 1.0 - fabs(tiltY);
		double xScale = (sqrtf(invTiltX * invTiltX + invTiltY * invTiltY) / sqrtf(2.0));
	
		double angle = calc_angle(B_ORIGIN, BPoint(tiltX, 0.0), BPoint(tiltX, tiltY), false);
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

	// configure pixel format
	pixelFormat.cover_scale(alpha);
	pixelFormat.solid(flags & FLAG_SOLID);

	// Create transformed ellipse vertex source and rasterize it.
	agg::ellipse ellipse(0.0, 0.0, radius, radius, 64);
	agg::conv_transform<agg::ellipse, Transformable> transformedEllipse(
		ellipse, ellipseTransform);
	rasterizer.add_path(transformedEllipse);

//bigtime_t renderTime = system_time();

	// special case for hardness = 1.0
	if (hardness == 1.0) {
		agg::render_scanlines(rasterizer, scanlineU, renderer);
	} else {
		// Brush gradient transformation
		Transformable gradientTransform = ellipseTransform;
		gradientTransform.invert();
	
		// Defining the brush gradient
		gradient_function	gradientFunction;
		interpolator_type	interpolator(gradientTransform);
		gradient_allocator	spanAllocator;
		color_array_type	array((agg::gray8*)sGaussTable);
		gradient_generator	gradientGenerator(spanAllocator, interpolator,
											  gradientFunction, array,
											  hardness * radius, radius * 2 - hardness * radius + 1.0);
		gradient_renderer	gradientRenderer(rendererBase, gradientGenerator);
	
		agg::render_scanlines(rasterizer, scanlineU, gradientRenderer);
	}

//bigtime_t finishTime = system_time();
//printf("init time: %lld, render time: %lld, total: %lld  (radius: %f, hardness: %f)\n",
//	   renderTime - startTime, finishTime - renderTime, finishTime - startTime, radius, hardness);
}

