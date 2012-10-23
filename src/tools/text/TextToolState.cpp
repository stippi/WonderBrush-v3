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
		Selection* selection, const BMessenger& configView)
	: TransformViewState(view)
	, fDocument(document)
	, fSelection(selection)

	, fConfigViewMessenger(configView)

	, fInsertionLayer(NULL)
	, fInsertionIndex(-1)
	, fText(NULL)
{
	// TODO: Find a way to change this later...
	SetInsertionInfo(fDocument->RootLayer(),
		fDocument->RootLayer()->CountObjects());

	fSelection->AddListener(this);
}

// destructor
TextToolState::~TextToolState()
{
	fSelection->RemoveListener(this);

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
	return NULL;
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

// ObjectSelected
void
TextToolState::ObjectSelected(const Selectable& selectable,
	const Selection::Controller* controller)
{
	if (controller == this) {
		// ignore changes triggered by ourself
		return;
	}

	Text* text = dynamic_cast<Text*>(selectable.Get());
	SetText(text);
}

// ObjectDeselected
void
TextToolState::ObjectDeselected(const Selectable& selectable,
	const Selection::Controller* controller)
{
	if (controller == this) {
		// ignore changes triggered by ourself
		return;
	}

	Text* text = dynamic_cast<Text*>(selectable.Get());
	if (text == fText)
		SetText(NULL);
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

// SetText
void
TextToolState::SetText(Text* text, bool modifySelection)
{
	if (fText == text)
		return;
	
	if (fText != NULL)
		fText->RemoveReference();

	fText = text;
	
	if (fText != NULL)
		fText->AddReference();
	
	if (text != NULL) {
		if (modifySelection)
			fSelection->Select(Selectable(text), this);
		SetObjectToCanvasTransformation(text->Transformation());
	} else {
		if (modifySelection)
			fSelection->DeselectAll(this);
		SetObjectToCanvasTransformation(Transformable());
	}

	_UpdateConfigView();
}

// SetString
void
TextToolState::SetString(const char* text)
{
	if (fText != NULL)
		fText->SetText(text);
}

// SetSize
void
TextToolState::SetSize(float size)
{
	if (fText != NULL)
		fText->SetFont(fText->getTextLayout().getFont().getName(), size);
}

// _UpdateConfigView
void
TextToolState::_UpdateConfigView() const
{
	if (!fConfigViewMessenger.IsValid())
		return;

	BMessage message(MSG_LAYOUT_CHANGED);

	if (fText != NULL) {
		message.AddFloat("size", fText->getTextLayout().getFont().getSize());
		message.AddString("text", fText->GetText());
	} else {
		message.AddString("text", "");
	}

	fConfigViewMessenger.SendMessage(&message);
}
