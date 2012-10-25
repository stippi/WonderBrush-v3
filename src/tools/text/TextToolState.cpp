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
#include "ui_defines.h"

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

	, fPickTextState(new(std::nothrow) PickTextState(this))
	, fCreateTextState(new(std::nothrow) CreateTextState(this))
	, fDragLeftTopState(new(std::nothrow) DragLeftTopState(this))
	, fDragWidthState(new(std::nothrow) DragWidthState(this))

	, fDocument(document)
	, fSelection(selection)

	, fConfigViewMessenger(configView)

	, fInsertionLayer(NULL)
	, fInsertionIndex(-1)
	, fText(NULL)
	
	, fCaretOffset(0)
	, fCaretAnchorX(0.0)
	, fShowCaret(true)
	, fCaretPulseRunner(NULL)
	
	, fStyle(new(std::nothrow) Style(), true)
	, fSize(12.0)
{
	// TODO: Find a way to change this later...
	SetInsertionInfo(fDocument->RootLayer(),
		fDocument->RootLayer()->CountObjects());

	fSelection->AddListener(this);

	fStyle.Get()->SetFillPaint(Paint(kBlack));
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

// #pragma mark -

// ModifiersChanged
void
TextToolState::ModifiersChanged(uint32 modifiers)
{
	DragStateViewState::ModifiersChanged(modifiers);
}

// HandleKeyDown
bool
TextToolState::HandleKeyDown(const StateView::KeyEvent& event,
	Command** _command)
{
	if (fText != NULL) {
		*_command = NULL;

		switch (event.key) {
			case B_UP_ARROW:
				_LineUp();
				break;
			case B_DOWN_ARROW:
				_LineDown();
				break;
			case B_LEFT_ARROW:
				_SetCaretOffset(fCaretOffset - 1, true);
				break;
			case B_RIGHT_ARROW:
				_SetCaretOffset(fCaretOffset + 1, true);
				break;

			case B_HOME:
				//_SetCaretOffset(0, true);
				_LineStart();
				break;
			case B_END:
				//_SetCaretOffset(fText->GetCharCount(), true);
				_LineEnd();
				break;

			case B_ENTER:
				Insert(fCaretOffset, "\n");
				break;
			case B_SPACE:
				Insert(fCaretOffset, " ");
				break;
			case B_TAB:
				Insert(fCaretOffset, " ");
				break;

			case B_ESCAPE:
				SetText(NULL, true);
				break;

			case B_BACKSPACE:
				if (fCaretOffset > 0)
					Remove(fCaretOffset - 1, 1);
				break;
			case B_DELETE:
				if (fCaretOffset < fText->GetCharCount())
					Remove(fCaretOffset, 1);
				break;
			case B_INSERT:
				// TODO: Toggle insert mode
				break;

			case B_PAGE_UP:
			case B_PAGE_DOWN:
			case B_SUBSTITUTE:
			case B_FUNCTION_KEY:
			case B_KATAKANA_HIRAGANA:
			case B_HANKAKU_ZENKAKU:
				break;

			default:
				if (event.bytes != NULL && event.length > 0) {
					// Handle null-termintating the string
					BString text(event.bytes, event.length);
					Insert(fCaretOffset, text.String());
				}
				break;
		}
		return true;
	}
	
	return DragStateViewState::HandleKeyDown(event, _command);
}

// HandleKeyUp
bool
TextToolState::HandleKeyUp(const StateView::KeyEvent& event,
	Command** _command)
{
	return DragStateViewState::HandleKeyUp(event, _command);
}

// #pragma mark -

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

	Text* text = new(std::nothrow) Text(kBlack);
	if (text == NULL) {
		fprintf(stderr, "TextToolState::CreateText(): Failed to allocate "
			"Text. Out of memory\n");
		return false;
	}

	text->SetWidth(0.0);
	
	text->SetText("Text", "DejaVuSerif.ttf", fSize, fStyle);
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
	fCaretAnchorX = 0.0;

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

// Insert
void
TextToolState::Insert(int32 textOffset, const char* text,
	bool setCaretOffset)
{
	if (fText != NULL) {
		fText->Insert(textOffset, text, "DejaVuSerif.ttf", fSize, fStyle);
		if (setCaretOffset)
			_SetCaretOffset(textOffset + BString(text).CountChars(), true);
		else
			UpdateBounds();
	}
}

// Remove
void
TextToolState::Remove(int32 textOffset, int32 length, bool setCaretOffset)
{
	if (fText != NULL) {
		fText->Remove(textOffset, length);
		if (setCaretOffset)
			_SetCaretOffset(textOffset, true);
		else
			UpdateBounds();
	}
}

// SetSize
void
TextToolState::SetSize(float size)
{
	fSize = size;
}

// SetSize
void
TextToolState::SetSize(float size, int32 textOffset, int32 length)
{
	fSize = size;
	if (fText != NULL) {
// TODO: Apply size to all fonts within range
//		fText->SetFont(fText->getTextLayout().getFont().getName(), size);
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
	_SetCaretOffset(startOffset, true);
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

// _LineStart
void
TextToolState::_LineStart()
{
	TextLayout& layout = fText->getTextLayout();
	
	int lineIndex = layout.getLineIndex(fCaretOffset);
	_SetCaretOffset(layout.getFirstOffsetOnLine(lineIndex), true);
}

// _LineEnd
void
TextToolState::_LineEnd()
{
	TextLayout& layout = fText->getTextLayout();
	
	int lineIndex = layout.getLineIndex(fCaretOffset);
	_SetCaretOffset(layout.getLastOffsetOnLine(lineIndex), true);
}

// _LineUp
void
TextToolState::_LineUp()
{
	TextLayout& layout = fText->getTextLayout();
	
	int lineIndex = layout.getLineIndex(fCaretOffset);
	_MoveToLine(layout, lineIndex - 1);
}

// _LineDown
void
TextToolState::_LineDown()
{
	TextLayout& layout = fText->getTextLayout();
	
	int lineIndex = layout.getLineIndex(fCaretOffset);
	_MoveToLine(layout, lineIndex + 1);
}

// _MoveToLine
void
TextToolState::_MoveToLine(TextLayout& layout, int32 lineIndex)
{
	if (lineIndex < 0 || lineIndex >= layout.getLineCount())
		return;
	
	double x1;
	double y1;
	double x2;
	double y2;
	layout.getLineBounds(lineIndex , &x1, &y1, &x2, &y2);
	
	bool rightOfCenter;
	int32 textOffset = layout.getOffset(fCaretAnchorX, (y1 + y2) / 2,
		rightOfCenter);

	if (rightOfCenter)
		textOffset++;

	_SetCaretOffset(textOffset, false);
}

// _SetCaretOffset
void
TextToolState::_SetCaretOffset(int32 offset, bool updateAnchor)
{
	if (offset < 0)
		offset = 0;
	if (offset > fText->GetCharCount())
		offset = fText->GetCharCount();

	if (offset == fCaretOffset)
		return;

	fCaretOffset = offset;
	fShowCaret = true;
	
	if (updateAnchor) {
		double x1;
		double y1;
		double x2;
		double y2;
		
		fText->getTextLayout().getTextBounds(fCaretOffset, x1, y1, x2, y2);
		fCaretAnchorX = x1;
	}
	
	UpdateBounds();
}

