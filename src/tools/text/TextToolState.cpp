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

// DragLeftTopState
class TextToolState::DragLeftTopState : public DragStateViewState::DragState {
public:
	DragLeftTopState(TextToolState* parent)
		: DragState(parent)
		, fParent(parent)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		fParent->TransformCanvasToObject(&origin);
		DragState::SetOrigin(origin);
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		BPoint objectCurrent = current;
		fParent->TransformCanvasToObject(&objectCurrent);
		
		BPoint leftTopOffset = objectCurrent - fOrigin;

		fParent->OffsetTextBy(leftTopOffset);

		fOrigin = current;
		fParent->TransformCanvasToObject(&fOrigin);
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		return BCursor(B_CURSOR_ID_MOVE);
	}

	virtual const char* CommandName() const
	{
		return "Move text";
	}

private:
	TextToolState*		fParent;
};

// DragWidthState
class TextToolState::DragWidthState : public DragStateViewState::DragState {
public:
	DragWidthState(TextToolState* parent)
		: DragState(parent)
		, fParent(parent)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		fParent->TransformCanvasToObject(&origin);
		DragState::SetOrigin(origin);
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		BPoint objectCurrent = current;
		fParent->TransformCanvasToObject(&objectCurrent);
		
//		BPoint leftTopOffset = objectCurrent - fOrigin;

//		fParent->SetWidth(leftTopOffset);

		fOrigin = current;
		fParent->TransformCanvasToObject(&fOrigin);
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		return BCursor(B_CURSOR_ID_MOVE);
	}

	virtual const char* CommandName() const
	{
		return "Change text width";
	}

private:
	TextToolState*		fParent;
};


// PickTextState
class TextToolState::PickTextState : public DragStateViewState::DragState {
public:
	PickTextState(TextToolState* parent)
		: DragState(parent)
		, fParent(parent)
		, fText(NULL)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		// Setup tool and switch to drag left/top state
		fParent->SetText(fText, true);

		if (fText == NULL)
			return;

		fParent->SetDragState(fParent->fDragLeftTopState);
		fParent->fDragLeftTopState->SetOrigin(origin);
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		// Never reached.
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		if (fText != NULL)
			return BCursor(B_CURSOR_ID_FOLLOW_LINK);
		return BCursor(B_CURSOR_SYSTEM_DEFAULT);
	}

	virtual const char* CommandName() const
	{
		return "Pick text";
	}

	void SetText(Text* text)
	{
		fText = text;
	}

private:
	TextToolState*		fParent;
	Text*				fText;
};

// CreateTextState
class TextToolState::CreateTextState : public DragStateViewState::DragState {
public:
	CreateTextState(TextToolState* parent)
		: DragState(parent)
		, fParent(parent)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		// Setup tool and switch to drag left/top state
		if (fParent->CreateText(origin)) {
			fParent->SetDragState(fParent->fDragLeftTopState);
			fParent->fDragLeftTopState->SetOrigin(origin);
		}
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		return BCursor(B_CURSOR_I_BEAM);
	}

	virtual const char* CommandName() const
	{
		return "Create text";
	}

private:
	TextToolState*		fParent;
};


// #pragma mark -


// constructor
TextToolState::TextToolState(StateView* view, Document* document,
		Selection* selection, const BMessenger& configView)
	: DragStateViewState(view)

	, fPickTextState(new (std::nothrow) PickTextState(this))
	, fCreateTextState(new (std::nothrow) CreateTextState(this))
	, fDragLeftTopState(new (std::nothrow) DragLeftTopState(this))
	, fDragWidthState(new (std::nothrow) DragWidthState(this))

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

	delete fPickTextState;
	delete fPickTextState;
	delete fDragLeftTopState;
	delete fDragWidthState;

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

// StartTransaction
Command*
TextToolState::StartTransaction(const char* commandName)
{
	return NULL;
}

// DragStateFor
TextToolState::DragState*
TextToolState::DragStateFor(BPoint canvasWhere, float zoomLevel) const
{
	if (fText != NULL) {
//		float inset = 7.0 / zoomLevel;
		
		BPoint where = canvasWhere;
		TransformCanvasToObject(&where);

		if (fText->Bounds().Contains(where))
			return fDragLeftTopState;
	}

	// If there is still no state, switch to the PickObjectsState
	// and try to find an object. If nothing is picked, unset on mouse down.
	Object* pickedObject = NULL;
	fDocument->RootLayer()->HitTest(canvasWhere, NULL, &pickedObject, true);
	
	Text* pickedText = dynamic_cast<Text*>(pickedObject);
	if (pickedText != NULL) {
		fPickTextState->SetText(pickedText);
		return fPickTextState;
	}
	
	return fCreateTextState;
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

// CreateText
bool
TextToolState::CreateText(BPoint canvasLocation)
{
	if (fInsertionLayer == NULL) {
		fprintf(stderr, "TextToolState::MouseDown(): No insertion layer "
			"specified\n");
		return false;
	}

	Text* text = new(std::nothrow) Text((rgb_color){ 0, 0, 0, 255 });
	if (text == NULL) {
		fprintf(stderr, "TextToolState::CreateText(): Failed to allocate "
			"Text. Out of memory\n");
		return false;
	}

	text->SetFont("DejaVuSerif.ttf", 12.0);
	text->SetWidth(200.0);
	text->SetText("Text");
	text->TranslateBy(canvasLocation);

	if (fInsertionIndex < 0)
		fInsertionIndex = 0;
	if (fInsertionIndex > fInsertionLayer->CountObjects())
		fInsertionIndex = fInsertionLayer->CountObjects();

	// TODO: Use Command
	if (!fInsertionLayer->AddObject(text, fInsertionIndex)) {
		fprintf(stderr, "TextToolState::CreateText(): Failed to add "
			"Text to Layer. Out of memory\n");
		text->RemoveReference();
		return false;
	}

	fInsertionIndex++;

	SetText(text, true);
	
	// Our reference to this object was transferred to the Layer
	
	return true;
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

// OffsetTextBy
void
TextToolState::OffsetTextBy(BPoint offset)
{
	if (fText == NULL)
		return;
	
	// TODO: Not correct...
	//fText->InverseTransform(&offset);
	fText->TranslateBy(offset);
	SetObjectToCanvasTransformation(fText->Transformation());
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