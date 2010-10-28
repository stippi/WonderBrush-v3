/*
 * Copyright 2009-2010 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "BrushToolState.h"

#include <Cursor.h>
#include <Shape.h>

#include <new>

#include <agg_math.h>

#include "BrushStroke.h"
#include "CommandStack.h"
#include "Document.h"
#include "Layer.h"
#include "support.h"

// constructor
BrushToolState::BrushToolState(StateView* view, Document* document,
		Selection* selection)
	: TransformViewState(view)
	, fDocument(document)
	, fSelection(selection)
	, fInsertionLayer(NULL)
	, fInsertionIndex(-1)
	, fBrushStroke(NULL)
{
	// TODO: Find a way to change this later...
	SetInsertionInfo(fDocument->RootLayer(),
		fDocument->RootLayer()->CountObjects());
}

// destructor
BrushToolState::~BrushToolState()
{
	SetInsertionInfo(NULL, -1);
}

// MessageReceived
bool
BrushToolState::MessageReceived(BMessage* message, Command** _command)
{
	bool handled = true;

	switch (message->what) {
		default:
			handled = TransformViewState::MessageReceived(message, _command);
	}

	return handled;
}

// MouseDown
void
BrushToolState::MouseDown(const MouseInfo& info)
{
	if (fBrushStroke != NULL)
		return;
	if (fInsertionLayer == NULL) {
		fprintf(stderr, "BrushToolState::MouseDown(): No insertion layer "
			"specified\n");
		return;
	}

	Brush* brush = new(std::nothrow) Brush(fBrush);
	if (brush == NULL) {
		fprintf(stderr, "BrushToolState::MouseDown(): Failed to allocate "
			"Brush. Out of memory\n");
		return;
	}

	fBrushStroke = new(std::nothrow)BrushStroke();
	if (fBrushStroke == NULL) {
		delete brush;
		fprintf(stderr, "BrushToolState::MouseDown(): Failed to allocate "
			"BrushStroke. Out of memory\n");
		return;
	}

	// transfer ownership of brush
	fBrushStroke->SetBrush(brush);
	brush->RemoveReference();

	if (!fInsertionLayer->AddObject(fBrushStroke, fInsertionIndex)) {
		delete fBrushStroke;
		fBrushStroke = NULL;
		fprintf(stderr, "BrushToolState::MouseDown(): Failed to add "
			"BrushStroke to Layer. Out of memory\n");
		return;
	}

	fInsertionIndex++;

	// Make sure the BrushStroke object can't go away while we intend to
	// still mess with it...
	fBrushStroke->AddReference();

	_AppendPoint(info);
}

// MouseMoved
void
BrushToolState::MouseMoved(const MouseInfo& info)
{
	_AppendPoint(info);
}

// MouseUp
Command*
BrushToolState::MouseUp()
{
	// TODO
	Command* command = NULL;

	if (fBrushStroke != NULL) {
		fBrushStroke->RemoveReference();
		fBrushStroke = NULL;
	}

	return command;
}

// Draw
void
BrushToolState::Draw(BView* view, BRect updateRect)
{
//	double scaleX;
//	double scaleY;
//	if (!EffectiveTransformation().GetAffineParameters(NULL, NULL, NULL,
//		&scaleX, &scaleY, NULL, NULL)) {
//		return;
//	}
//
//	scaleX *= fView->ZoomLevel();
//	scaleY *= fView->ZoomLevel();
//
//	view->PushState();
//
//	view->MovePenTo(B_ORIGIN);
//
//	BShape shape;
//
//	view->SetHighColor(255, 255, 255);
//	view->FillShape(&shape);
//
//	shape.Clear();
//
//	view->PopState();
}

// Bounds
BRect
BrushToolState::Bounds() const
{
	return BRect(0, 0, -1, -1);
}

// #pragma mark -

// SetInsertionInfo
void
BrushToolState::SetInsertionInfo(Layer* layer, int32 index)
{
	if (layer != fInsertionLayer) {
		if (layer != NULL)
			layer->AddReference();
		if (fInsertionLayer != NULL)
			fInsertionLayer->RemoveReference();
		fInsertionLayer = layer;
	}
	fInsertionIndex = index;
}

// _AppendPoint
void
BrushToolState::_AppendPoint(const MouseInfo& info)
{
	if (fBrushStroke == NULL)
		return;

	StrokePoint point(info.position, info.pressure, info.tilt.x, info.tilt.y);
	fBrushStroke->AppendPoint(point);
}

