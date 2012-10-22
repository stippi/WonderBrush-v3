/*
 * Copyright 2012 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "TextToolState.h"

#include <Cursor.h>
#include <Shape.h>

#include <new>

#include <agg_math.h>

#include "CommandStack.h"
#include "Document.h"
#include "Layer.h"
#include "ObjectAddedCommand.h"
#include "support.h"
#include "Text.h"

// constructor
TextToolState::TextToolState(StateView* view, Document* document,
		Selection* selection)
	: TransformViewState(view)
	, fDocument(document)
	, fSelection(selection)
	, fInsertionLayer(NULL)
	, fInsertionIndex(-1)
	, fText(NULL)
{
	// TODO: Find a way to change this later...
	SetInsertionInfo(fDocument->RootLayer(),
		fDocument->RootLayer()->CountObjects());
}

// destructor
TextToolState::~TextToolState()
{
	SetInsertionInfo(NULL, -1);
}

// MessageReceived
bool
TextToolState::MessageReceived(BMessage* message, Command** _command)
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
TextToolState::MouseDown(const MouseInfo& info)
{
	if (fText != NULL)
		return;
	if (fInsertionLayer == NULL) {
		fprintf(stderr, "TextToolState::MouseDown(): No insertion layer "
			"specified\n");
		return;
	}

//	Brush* brush = new(std::nothrow) Brush(fBrush);
//	if (brush == NULL) {
//		fprintf(stderr, "TextToolState::MouseDown(): Failed to allocate "
//			"Brush. Out of memory\n");
//		return;
//	}
//
//	fBrushStroke = new(std::nothrow)BrushStroke();
//	if (fBrushStroke == NULL) {
//		fprintf(stderr, "TextToolState::MouseDown(): Failed to allocate "
//			"BrushStroke. Out of memory\n");
//		delete brush;
//		return;
//	}
//
//	// transfer ownership of brush
//	fBrushStroke->SetBrush(brush);
//	brush->RemoveReference();
//
//	if (fInsertionIndex < 0)
//		fInsertionIndex = 0;
//	if (fInsertionIndex > fInsertionLayer->CountObjects())
//		fInsertionIndex = fInsertionLayer->CountObjects();
//
//	if (!fInsertionLayer->AddObject(fBrushStroke, fInsertionIndex)) {
//		fprintf(stderr, "TextToolState::MouseDown(): Failed to add "
//			"BrushStroke to Layer. Out of memory\n");
//		fBrushStroke->RemoveReference();
//		fBrushStroke = NULL;
//		return;
//	}
//
//	fInsertionIndex++;
}

// MouseMoved
void
TextToolState::MouseMoved(const MouseInfo& info)
{
}

// MouseUp
Command*
TextToolState::MouseUp()
{
	if (fText == NULL)
		return NULL;

	Command* command = new(std::nothrow) ObjectAddedCommand(fText,
		fSelection);

	fText->RemoveReference();
	fText = NULL;

	return command;
}

// Draw
void
TextToolState::Draw(BView* view, BRect updateRect)
{
}

// Bounds
BRect
TextToolState::Bounds() const
{
	return BRect(0, 0, -1, -1);
}

// #pragma mark -

// SetInsertionInfo
void
TextToolState::SetInsertionInfo(Layer* layer, int32 index)
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
