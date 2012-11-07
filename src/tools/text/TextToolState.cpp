#ifndef _TEXTTOOLSTATE_CPP_
#define _TEXTTOOLSTATE_CPP_

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
#include "CurrentColor.h"
#include "Document.h"
#include "FontCache.h"
#include "InsertTextCommand.h"
#include "Layer.h"
#include "ObjectAddedCommand.h"
#include "RemoveTextCommand.h"
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

// DragCaretState
class TextToolState::DragCaretState : public DragStateViewState::DragState {
public:
	DragCaretState(TextToolState* parent)
		: DragState(parent)
		, fParent(parent)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		fParent->TransformCanvasToObject(&origin);
		DragState::SetOrigin(origin);

		bool select = (fParent->Modifiers() & B_SHIFT_KEY) != 0;
		fParent->SetCaret(origin, select);
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		fParent->TransformCanvasToObject(&current);

		fParent->SetCaret(current, true);
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		return BCursor(B_CURSOR_ID_I_BEAM);
	}

	virtual const char* CommandName() const
	{
		return "Select text";
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

		fParent->SetDragState(fParent->fDragCaretState);
		fParent->fDragCaretState->SetOrigin(origin);
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
		return BCursor(B_CURSOR_ID_SYSTEM_DEFAULT);
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
		Selection* selection, CurrentColor* color,
		const BMessenger& configView)
	: DragStateViewState(view)

	, fPickTextState(new(std::nothrow) PickTextState(this))
	, fCreateTextState(new(std::nothrow) CreateTextState(this))
	, fDragLeftTopState(new(std::nothrow) DragLeftTopState(this))
	, fDragWidthState(new(std::nothrow) DragWidthState(this))
	, fDragCaretState(new(std::nothrow) DragCaretState(this))

	, fDocument(document)
	, fSelection(selection)
	, fCurrentColor(color)

	, fConfigViewMessenger(configView)

	, fInsertionLayer(NULL)
	, fInsertionIndex(-1)
	, fText(NULL)

	, fCaretOffset(0)
	, fShowCaret(true)
	, fCaretAnchorX(0.0)
	, fCaretPulseRunner(NULL)

	, fStyle(new(std::nothrow) Style(), true)
	, fFontFamily("DejaVu Serif")
	, fFontStyle("Book")
	, fSize(12.0)
{
	// TODO: Find a way to change this later...
	SetInsertionInfo(fDocument->RootLayer(),
		fDocument->RootLayer()->CountObjects());

	fSelection->AddListener(this);
	fCurrentColor->AddListener(this);

	fStyle.Get()->SetFillPaint(Paint(fCurrentColor->Color()));
}

// destructor
TextToolState::~TextToolState()
{
	SetText(NULL);
	fCurrentColor->RemoveListener(this);
	fSelection->RemoveListener(this);

	delete fPickTextState;
	delete fCreateTextState;
	delete fDragLeftTopState;
	delete fDragWidthState;
	delete fDragCaretState;

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
			&pulseMessage, 500000LL);
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

		bool select = (event.modifiers & B_SHIFT_KEY) != 0;

		switch (event.key) {
			case B_UP_ARROW:
				_LineUp(select);
				break;
			case B_DOWN_ARROW:
				_LineDown(select);
				break;
			case B_LEFT_ARROW:
				if (_HasSelection() && !select) {
					_SetCaretOffset(
						min_c(fCaretOffset, fSelectionAnchorOffset),
						true, false, true
					);
				} else
					_SetCaretOffset(fCaretOffset - 1, true, select, true);
				break;
			case B_RIGHT_ARROW:
				if (_HasSelection() && !select) {
					_SetCaretOffset(
						max_c(fCaretOffset, fSelectionAnchorOffset),
						true, false, true
					);
				} else
					_SetCaretOffset(fCaretOffset + 1, true, select, true);
				break;

			case B_HOME:
				//_SetCaretOffset(0, true);
				_LineStart(select);
				break;
			case B_END:
				//_SetCaretOffset(fText->GetCharCount(), true);
				_LineEnd(select);
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
				if (_HasSelection()) {
					Remove(_SelectionStart(), _SelectionLength());
				} else {
					if (fCaretOffset > 0)
						Remove(fCaretOffset - 1, 1);
				}
				break;
			case B_DELETE:
				if (_HasSelection()) {
					Remove(_SelectionStart(), _SelectionLength());
				} else {
					if (fCaretOffset < fText->GetCharCount())
						Remove(fCaretOffset, 1);
				}
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
	if (fSelectionAnchorOffset == fCaretOffset) {
		if (fShowCaret)
			_DrawCaret(view, fCaretOffset);
	} else {
		if (fSelectionAnchorOffset < fCaretOffset)
			_DrawSelection(view, fSelectionAnchorOffset, fCaretOffset);
		else
			_DrawSelection(view, fCaretOffset, fSelectionAnchorOffset);
	}
}

// Bounds
BRect
TextToolState::Bounds() const
{
	if (fText == NULL)
		return BRect(0, 0, -1, -1);

	BRect bounds = fText->Bounds();
	bounds.right = fText->Width();
	bounds.InsetBy(-15, -15);
	TransformObjectToView(&bounds);
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

		BPoint widthOffset(fText->Width(), -10.0f / scaleY / zoomLevel);
		if (point_point_distance(objectWhere, widthOffset) < inset)
			return fDragWidthState;

		BPoint leftTop(0.0, -10.0f / scaleY / zoomLevel);
		if (point_point_distance(objectWhere, leftTop) < inset)
			return fDragLeftTopState;

		BRect bounds = fText->Bounds();
		bounds.top -= 10.0f / scaleY / zoomLevel;
		bounds.InsetBy(-inset, -inset);

		if (bounds.Contains(objectWhere))
			return fDragCaretState;
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

// ObjectChanged
void
TextToolState::ObjectChanged(const Notifier* object)
{
	if (fText != NULL && object == fText) {
		SetObjectToCanvasTransformation(fText->Transformation());
		UpdateBounds();
		UpdateDragState();
		_UpdateConfigView();
	}
	if (object == fCurrentColor) {
		SetColor(fCurrentColor->Color());
	}
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

	BString initialText = fNextText;
	if (initialText.Length() == 0)
		initialText = "Text";

	text->SetText(initialText.String(), Font(fFontFamily, fFontStyle, fSize),
		fStyle);
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

	View()->PerformCommand(new(std::nothrow) ObjectAddedCommand(text,
		fSelection));


	return true;
}

// SetText
void
TextToolState::SetText(Text* text, bool modifySelection)
{
	if (fText == text)
		return;

	if (fText != NULL) {
		fText->RemoveListener(this);
		fText->RemoveReference();
	}

	fText = text;

	if (fText != NULL) {
		fText->AddReference();
		fText->AddListener(this);
	}

	if (text != NULL) {
		if (modifySelection)
			fSelection->Select(Selectable(text), this);
		SetObjectToCanvasTransformation(text->Transformation());
	} else {
		if (modifySelection)
			fSelection->DeselectAll(this);
		SetObjectToCanvasTransformation(Transformable());
	}

	fNextText = "";
	fCaretOffset = 0;
	fCaretAnchorX = 0.0;

	_UpdateConfigView();
	_UpdateConfigViewSelection();
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
		if (setCaretOffset && _HasSelection()) {
			int32 start = _SelectionStart();
//			fText->Remove(start, _SelectionLength());
			View()->PerformCommand(
				new(std::nothrow) RemoveTextCommand(
					fText, start, _SelectionLength()
				)
			);
			textOffset = start;
		}

//		fText->Insert(textOffset, text, Font(fFontFamily, fFontStyle, fSize),
//			fStyle);
		View()->PerformCommand(
			new(std::nothrow) InsertTextCommand(
				fText, textOffset, text, Font(fFontFamily, fFontStyle, fSize),
				fStyle
			)
		);

		if (setCaretOffset) {
			_SetCaretOffset(textOffset + BString(text).CountChars(), true,
				false, false);
		} else
			UpdateBounds();
	} else {
		fNextText.InsertChars(text, textOffset);
	}
}

// Remove
void
TextToolState::Remove(int32 textOffset, int32 length, bool setCaretOffset)
{
	if (fText != NULL) {
//		fText->Remove(textOffset, length);
		View()->PerformCommand(
			new(std::nothrow) RemoveTextCommand(fText, textOffset, length)
		);
		if (setCaretOffset) {
			_SetCaretOffset(textOffset, true, false, true);
		} else
			UpdateBounds();
	} else {
		fNextText.RemoveChars(textOffset, length);
	}
}

// SetFont
void
TextToolState::SetFont(const char* family, const char* style)
{
	fFontFamily = family;
	fFontStyle = style;
	if (_HasSelection()) {
		fText->SetFont(_SelectionStart(), _SelectionLength(),
			family, style);
		UpdateBounds();
	}
}

// SetSize
void
TextToolState::SetSize(float size)
{
	fSize = size;
	if (_HasSelection()) {
		fText->SetSize(_SelectionStart(), _SelectionLength(), size);
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

// SetColor
void
TextToolState::SetColor(const rgb_color& color)
{
	::Style* style = new(std::nothrow) ::Style();
	if (style == NULL)
		return;

	style->SetFillPaint(Paint(color));
	fStyle.SetTo(style, true);

	if (_HasSelection()) {
		fText->SetColor(_SelectionStart(), _SelectionLength(), color);
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
	if (fText != NULL) {
		_SetCaretOffset(startOffset, false, false, false);
		_SetCaretOffset(endOffset, true, true, true);
	}
}

// SetCaret
void
TextToolState::SetCaret(const BPoint& location, bool select)
{
	if (fText == NULL)
		return;

	bool rightOfChar = false;
	int32 caretOffset = fText->getTextLayout().getOffset(location.x, location.y,
		rightOfChar);

	if (rightOfChar)
		caretOffset++;

	_SetCaretOffset(caretOffset, true, select, true);
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
		message.AddString("family", fFontFamily);
		message.AddString("style", fFontStyle);
		message.AddFloat("size", fSize);
		message.AddString("text", fText->GetText());
	} else {
		message.AddString("text", "");
	}

	fConfigViewMessenger.SendMessage(&message);
}

// _UpdateConfigViewSelection
void
TextToolState::_UpdateConfigViewSelection() const
{
	if (!fConfigViewMessenger.IsValid())
		return;

	BMessage message(MSG_SET_SELECTION);

	message.AddInt32("selection start", _SelectionStart());
	message.AddInt32("selection end", _SelectionEnd());
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

	_DrawInvertedShape(view, shape);
}

// _DrawSelection
void
TextToolState::_DrawSelection(BView* view, int32 startOffset, int32 endOffset)
{
	BShape shape;
	_GetSelectionShape(fText->getTextLayout(), shape, startOffset, endOffset);
	_DrawInvertedShape(view, shape);
}

// _DrawInvertedShape
void
TextToolState::_DrawInvertedShape(BView* view, BShape& shape)
{
	view->PushState();
	uint32 flags = view->Flags();
	view->SetFlags(flags | B_SUBPIXEL_PRECISE);
	view->SetDrawingMode(B_OP_INVERT);
	view->MovePenTo(B_ORIGIN);
	view->FillShape(&shape);
	view->SetFlags(flags);
	view->PopState();
}

// #pragma mark -

// _LineStart
void
TextToolState::_LineStart(bool select)
{
	TextLayout& layout = fText->getTextLayout();

	int lineIndex = layout.getLineIndex(fCaretOffset);
	_SetCaretOffset(layout.getFirstOffsetOnLine(lineIndex), true, select,
		true);
}

// _LineEnd
void
TextToolState::_LineEnd(bool select)
{
	TextLayout& layout = fText->getTextLayout();

	int lineIndex = layout.getLineIndex(fCaretOffset);
	_SetCaretOffset(layout.getLastOffsetOnLine(lineIndex), true, select,
		true);
}

// _LineUp
void
TextToolState::_LineUp(bool select)
{
	TextLayout& layout = fText->getTextLayout();

	int lineIndex = layout.getLineIndex(fCaretOffset);
	_MoveToLine(layout, lineIndex - 1, select);
}

// _LineDown
void
TextToolState::_LineDown(bool select)
{
	TextLayout& layout = fText->getTextLayout();

	int lineIndex = layout.getLineIndex(fCaretOffset);
	_MoveToLine(layout, lineIndex + 1, select);
}

// _MoveToLine
void
TextToolState::_MoveToLine(TextLayout& layout, int32 lineIndex, bool select)
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

	_SetCaretOffset(textOffset, false, select, true);
}

// _SetCaretOffset
void
TextToolState::_SetCaretOffset(int32 offset, bool updateAnchor,
	bool lockSelectionAnchor, bool updateSelectionStyle)
{
	if (offset < 0)
		offset = 0;
	if (offset > fText->GetCharCount())
		offset = fText->GetCharCount();

	if (offset == fCaretOffset && (lockSelectionAnchor
			|| offset == fSelectionAnchorOffset)) {
		return;
	}

	if (!lockSelectionAnchor)
		fSelectionAnchorOffset = offset;

	fCaretOffset = offset;
	fShowCaret = true;

	_UpdateConfigViewSelection();

	if (updateAnchor) {
		double x1;
		double y1;
		double x2;
		double y2;

		fText->getTextLayout().getTextBounds(fCaretOffset, x1, y1, x2, y2);
		fCaretAnchorX = x1;
	}

	if (updateSelectionStyle)
		_AdoptStyleAtOffset(fCaretOffset - 1);

	UpdateBounds();
}

// _GetSelectionShape
void
TextToolState::_GetSelectionShape(TextLayout& layout, BShape& shape,
	int32 start, int32 end) const
{
	double startX1;
	double startY1;
	double startX2;
	double startY2;
	layout.getTextBounds(start, startX1, startY1, startX2, startY2);

	double endX1;
	double endY1;
	double endX2;
	double endY2;
	layout.getTextBounds(end, endX1, endY1, endX2, endY2);

	int32 startLineIndex = layout.getLineIndex(start);
	int32 endLineIndex = layout.getLineIndex(end);

	if (startLineIndex == endLineIndex) {
		// Selection on one line
		BPoint lt(startX1, startY1);
		BPoint rt(endX1, endY1);
		BPoint rb(endX1, endY2);
		BPoint lb(startX1, startY2);

		TransformObjectToView(&lt, false);
		TransformObjectToView(&rt, false);
		TransformObjectToView(&rb, false);
		TransformObjectToView(&lb, false);

		shape.MoveTo(lt);
		shape.LineTo(rt);
		shape.LineTo(rb);
		shape.LineTo(lb);
		shape.Close();
	} else {
		// Selection over multiple lines
		BPoint p;
		p = BPoint(startX1, startY1);
		TransformObjectToView(&p, false);
		shape.MoveTo(p);

		p = BPoint(layout.getWidth(), startY1);
		TransformObjectToView(&p, false);
		shape.LineTo(p);

		p = BPoint(layout.getWidth(), endY1);
		TransformObjectToView(&p, false);
		shape.LineTo(p);

		p = BPoint(endX1, endY1);
		TransformObjectToView(&p, false);
		shape.LineTo(p);

		p = BPoint(endX1, endY2);
		TransformObjectToView(&p, false);
		shape.LineTo(p);

		p = BPoint(0, endY2);
		TransformObjectToView(&p, false);
		shape.LineTo(p);

		p = BPoint(0, startY2);
		TransformObjectToView(&p, false);
		shape.LineTo(p);

		p = BPoint(startX1, startY2);
		TransformObjectToView(&p, false);
		shape.LineTo(p);

		shape.Close();
	}
}

// _HasSelection
bool
TextToolState::_HasSelection() const
{
	return fText != NULL && fCaretOffset != fSelectionAnchorOffset;
}

// _SelectionStart
int32
TextToolState::_SelectionStart() const
{
	return min_c(fCaretOffset, fSelectionAnchorOffset);
}

// _SelectionEnd
int32
TextToolState::_SelectionEnd() const
{
	return max_c(fCaretOffset, fSelectionAnchorOffset);
}

// _SelectionLength
int32
TextToolState::_SelectionLength() const
{
	return fabs(fCaretOffset - fSelectionAnchorOffset);
}

// _AdoptStyleAtOffset
void
TextToolState::_AdoptStyleAtOffset(int32 textOffset)
{
	if (fText == NULL)
		return;

	if (textOffset < 0)
		textOffset = 0;

	TextLayout& layout = fText->getTextLayout();

	Font font(layout.getFont());
	Color fgColor;
	bool strikeOut;
	Color strikeColor;
	bool underline;
	unsigned underlineStyle;
	Color underlineColor;

	if (!layout.getInfo(textOffset, font, fgColor, strikeOut,
			strikeColor, underline, underlineStyle, underlineColor)) {
		return;
	}

	if (fSize != font.getSize() || fFontFamily != font.getFamily()
		|| fFontStyle != font.getStyle()) {
		fSize = font.getSize();
		fFontFamily = font.getFamily();
		fFontStyle = font.getStyle();

		_UpdateConfigView();
	}
}


#endif	// _TEXTTOOLSTATE_CPP_
