/*
 * Copyright 2012 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "TextToolState.h"

#include <Cursor.h>
#include <MessageRunner.h>
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
		BPoint widthOffset(fParent->Width(), 0);
		DragState::SetOrigin(widthOffset - origin);
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		BPoint objectCurrent = current;
		fParent->TransformCanvasToObject(&objectCurrent);
		
		objectCurrent += fOrigin;
		
		if (objectCurrent.x < 0)
			objectCurrent.x = 0;
		
		fParent->SetWidth(objectCurrent.x);
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		return BCursor(B_CURSOR_ID_RESIZE_EAST_WEST);
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
		return BCursor(B_CURSOR_ID_I_BEAM);
	}

	virtual const char* CommandName() const
	{
		return "Create text";
	}

private:
	TextToolState*		fParent;
};


// #pragma mark -


enum {
	MSG_CARET_PULSE				= 'plse',
};

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
	
	, fCaretOffset(0)
	, fShowCaret(true)
	, fCaretPulseRunner(NULL)
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

	delete fCaretPulseRunner;
}

// Init
void
TextToolState::Init()
{
	DragStateViewState::Init();

	if (fCaretPulseRunner == NULL) {
		BMessage pulseMessage(MSG_CARET_PULSE);
		fCaretPulseRunner = new BMessageRunner(BMessenger(fView),
			&pulseMessage, 1000000LL);
	}
}

// Cleanup
void
TextToolState::Cleanup()
{
	delete fCaretPulseRunner;
	fCaretPulseRunner = NULL;

	DragStateViewState::Cleanup();
}

// MessageReceived
bool
TextToolState::MessageReceived(BMessage* message, Command** _command)
{
	bool handled = true;

	switch (message->what) {
		case MSG_CARET_PULSE:
			fShowCaret = !fShowCaret;
			UpdateBounds();
			break;
		default:
			handled = TransformViewState::MessageReceived(message, _command);
	}

	return handled;
}

// Draw
void
TextToolState::Draw(BView* view, BRect updateRect)
{
	if (fText == NULL)
		return;
	
	_DrawControls(view);
	
	if (fShowCaret)
		_DrawCaret(view, fCaretOffset);
}

// Bounds
BRect
TextToolState::Bounds() const
{
	if (fText == NULL)
		return BRect(0, 0, -1, -1);

	BRect bounds = fText->TransformedBounds();
	bounds.InsetBy(-15, -15);
	return bounds;
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
	double scaleX;
	double scaleY;
	if (fText != NULL
		&& fText->GetAffineParameters(NULL, NULL, NULL,
			&scaleX, &scaleY, NULL, NULL)) {
		float inset = 7.0 / zoomLevel;
		
		BPoint objectWhere = canvasWhere;
		TransformCanvasToObject(&objectWhere);

		BPoint widthOffset(fText->Width(), -10.0f / scaleY);
		if (point_point_distance(objectWhere, widthOffset) < inset)
			return fDragWidthState;

		BRect bounds = fText->Bounds();
		bounds.top -= 10.0f / scaleY;
		bounds.InsetBy(-inset, -inset);

		if (bounds.Contains(objectWhere))
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
	text->SetWidth(0.0);
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

	fCaretOffset = 0;

	_UpdateConfigView();
}

// OffsetTextBy
void
TextToolState::OffsetTextBy(BPoint offset)
{
	if (fText != NULL) {
		// TODO: Not correct...
		fText->TranslateBy(offset);
		SetObjectToCanvasTransformation(fText->Transformation());
		UpdateBounds();
	}
}

// SetString
void
TextToolState::SetString(const char* text)
{
	if (fText != NULL) {
		fText->SetText(text);
		UpdateBounds();
	}
}

// SetSize
void
TextToolState::SetSize(float size)
{
	if (fText != NULL) {
		fText->SetFont(fText->getTextLayout().getFont().getName(), size);
		UpdateBounds();
	}
}

// SetWidth
void
TextToolState::SetWidth(float width)
{
	if (fText != NULL) {
		fText->SetWidth(width);
		UpdateBounds();
	}
}

// Width
float
TextToolState::Width() const
{
	if (fText != NULL)
		return fText->Width();
	return 0.0f;
}

// SelectionChanged
void
TextToolState::SelectionChanged(int32 startOffset, int32 endOffset)
{
	fCaretOffset = startOffset;
	fShowCaret = true;
	UpdateBounds();
}

// #pragma mark - private

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

// _DrawControls
void
TextToolState::_DrawControls(BView* view)
{
	double scaleX;
	double scaleY;
	if (!EffectiveTransformation().GetAffineParameters(NULL, NULL, NULL,
		&scaleX, &scaleY, NULL, NULL)) {
		return;
	}

	scaleX *= fView->ZoomLevel();
	scaleY *= fView->ZoomLevel();

	BPoint origin(0.0f, -10.0f / scaleY);
	TransformObjectToView(&origin, true);

	BPoint widthOffset(fText->Width(), -10.0f / scaleY);
	TransformObjectToView(&widthOffset, true);

	view->SetHighColor(0, 0, 0, 200);
	view->SetPenSize(3.0);
	view->StrokeLine(origin, widthOffset);
	view->SetHighColor(255, 255, 255, 200);
	view->SetPenSize(1.0);
	view->StrokeLine(origin, widthOffset);

	float size = 3;
	view->SetHighColor(255, 255, 255, 200);
	view->FillEllipse(BRect(origin.x - size, origin.y - size,
		origin.x + size + 0.5, origin.y + size + 0.5));
	view->SetHighColor(0, 0, 0, 200);
	view->StrokeEllipse(BRect(origin.x - size, origin.y - size,
		origin.x + size + 1, origin.y + size + 1));

	size = 2;
	view->SetHighColor(255, 255, 255, 200);
	view->FillRect(BRect(widthOffset.x - size, widthOffset.y - size,
		widthOffset.x + size + 0.5, widthOffset.y + size + 0.5));
	view->SetHighColor(0, 0, 0, 200);
	view->StrokeRect(BRect(widthOffset.x - size, widthOffset.y - size,
		widthOffset.x + size + 1, widthOffset.y + size + 1));
}

// _DrawCaret
void
TextToolState::_DrawCaret(BView* view, int32 textOffset)
{
	double x1;
	double y1;
	double x2;
	double y2;
	
	fText->getTextLayout().getTextBounds(textOffset, x1, y1, x2, y2);
	x2 = x1 + 2;

	BPoint lt(x1, y1);
	BPoint rt(x2, y1);
	BPoint lb(x1, y2);
	BPoint rb(x2, y2);

	TransformObjectToView(&lt, false);
	TransformObjectToView(&rt, false);
	TransformObjectToView(&lb, false);
	TransformObjectToView(&rb, false);

	BShape shape;
	shape.MoveTo(lt);
	shape.LineTo(rt);
	shape.LineTo(rb);
	shape.LineTo(lb);
	shape.Close();

	view->PushState();
	uint32 flags = view->Flags();
	view->SetFlags(flags | B_SUBPIXEL_PRECISE);
	view->SetDrawingMode(B_OP_INVERT);
	view->MovePenTo(B_ORIGIN);
	view->FillShape(&shape);
	view->SetFlags(flags);
	view->PopState();
}

