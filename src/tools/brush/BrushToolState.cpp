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
	if (!fDocument->WriteLock())
		return;

	// TODO

	fDocument->WriteUnlock();
}

// MouseMoved
void
BrushToolState::MouseMoved(const MouseInfo& info)
{
	if (!fDocument->WriteLock())
		return;

	// TODO

	fDocument->WriteUnlock();
}

// MouseUp
Command*
BrushToolState::MouseUp()
{
	if (!fDocument->WriteLock())
		return NULL;

	// TODO
	Command* command = NULL;

	fDocument->WriteUnlock();

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

