/*
 * Copyright 2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "TransformViewState.h"


// constructor
TransformViewState::TransformViewState(StateView* view)
	:
	ViewState(view),
	fObjectToCanvasTransformation(),
	fAdditionalTransformation(),
	fEffectiveTransformation()
{
}

// destructor
TransformViewState::~TransformViewState()
{
}

// SetObjectToCanvasTransformation
void
TransformViewState::SetObjectToCanvasTransformation(
	const Transformable& transformation)
{
	fObjectToCanvasTransformation = transformation;
	_UpdateTransformation();
}

// SetAdditionalTransformation
void
TransformViewState::SetAdditionalTransformation(
	const Transformable& transformation)
{
	fAdditionalTransformation = transformation;
	_UpdateTransformation();
}

// TransformObjectToCanvas
void
TransformViewState::TransformObjectToCanvas(BPoint* point) const
{
	fEffectiveTransformation.Transform(point);
}

// TransformCanvasToObject
void
TransformViewState::TransformCanvasToObject(BPoint* point) const
{
	fEffectiveTransformation.InverseTransform(point);
}

// TransformObjectToView
void
TransformViewState::TransformObjectToView(BPoint* point) const
{
	TransformObjectToCanvas(point);
	TransformCanvasToView(point);
}

// TransformViewToObject
void
TransformViewState::TransformViewToObject(BPoint* point) const
{
	TransformViewToCanvas(point);
	TransformCanvasToObject(point);
}

// TransformObjectToCanvas
void
TransformViewState::TransformObjectToCanvas(BRect* bounds) const
{
	*bounds = fEffectiveTransformation.TransformBounds(*bounds);
}

// TransformObjectToView
void
TransformViewState::TransformObjectToView(BRect* bounds) const
{
	TransformObjectToCanvas(bounds);
	TransformCanvasToView(bounds);
}

// ViewspaceRotation
float
TransformViewState::ViewspaceRotation() const
{
	double rotation;
	fEffectiveTransformation.GetAffineParameters(NULL, NULL, &rotation,
		NULL, NULL, NULL, NULL);
	// TODO: If ever we can rotate the canvas on the view, this rotation
	// needs to be added here!
	return (float)(rotation * 180.0 / PI);
}

// #pragma mark -

// _UpdateTransformation
void
TransformViewState::_UpdateTransformation()
{
	fEffectiveTransformation = fObjectToCanvasTransformation;
	fEffectiveTransformation.Multiply(fAdditionalTransformation);
}

