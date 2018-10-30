#ifndef _TEXTTOOLSTATE_CPP_
#define _TEXTTOOLSTATE_CPP_

/*
 * Copyright 2012-2018 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "TextToolState.h"
#include "TextToolStatePlatformDelegate.h"

#include <Clipboard.h>
#include <Cursor.h>
#include <MessageRunner.h>
#include <Shape.h>

#include <new>

#include <agg_math.h>

#include "EditManager.h"
#include "CompoundEdit.h"
#include "CurrentColor.h"
#include "Document.h"
#include "FontCache.h"
#include "InsertTextEdit.h"
#include "Layer.h"
#include "ObjectAddedEdit.h"
#include "RemoveTextEdit.h"
#include "support.h"
#include "SetTextAlignmentEdit.h"
#include "SetTextStyleEdit.h"
#include "SetTextWidthEdit.h"
#include "Text.h"
#include "TransformObjectEdit.h"
#include "ui_defines.h"

enum {
	MSG_TEXT_CHANGED	= 'tttc',
};

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
		DragState::SetOrigin(origin);
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		BPoint objectCurrent = current;
		fParent->TransformCanvasToObject(&objectCurrent);

		BPoint objectOrigin = fOrigin;
		fParent->TransformCanvasToObject(&objectOrigin);

		BPoint leftTopOffset = objectCurrent - objectOrigin;
		fParent->OffsetTextBy(leftTopOffset);

		fOrigin = current;
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
		return BCursor(B_CURSOR_ID_SYSTEM_DEFAULT);
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


// DeselectTextState
class TextToolState::DeselectTextState : public DragStateViewState::DragState {
public:
	DeselectTextState(TextToolState* parent)
		: DragState(parent)
		, fParent(parent)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		fParent->SetText(NULL, true);
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
		return "Deselect text";
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

	, fPlatformDelegate(new PlatformDelegate(this))

	, fPickTextState(new(std::nothrow) PickTextState(this))
	, fCreateTextState(new(std::nothrow) CreateTextState(this))
	, fDeselectTextState(new(std::nothrow) DeselectTextState(this))
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

	, fSelectionAnchorOffset(0)
	, fCaretOffset(0)
	, fShowCaret(true)
	, fCaretAnchorX(0.0)
	, fCaretPulseRunner(NULL)

	, fStyle(new(std::nothrow) Style(), true)
	, fFontFamily("DejaVu Serif")
	, fFontStyle("Book")
	, fSize(12.0)
	, fTextAlignment(TEXT_ALIGNMENT_LEFT)
	, fGlyphSpacing(0.0)
	, fFauxWeight(0.0)
	, fFauxItalic(0.0)

	, fIgnoreColorColorNotifiactions(false)
{
	// TODO: Find a way to change this later...
	SetInsertionInfo(fDocument->RootLayer(),
		fDocument->RootLayer()->CountObjects());

	fCurrentColor->AddListener(this);

	fStyle->FillPaint()->SetColor(fCurrentColor->Color());
}

// destructor
TextToolState::~TextToolState()
{
	SetText(NULL);
	fCurrentColor->RemoveListener(this);
	fSelection->RemoveListener(this);

	delete fPickTextState;
	delete fCreateTextState;
	delete fDeselectTextState;
	delete fDragLeftTopState;
	delete fDragWidthState;
	delete fDragCaretState;

	SetInsertionInfo(NULL, -1);

	delete fCaretPulseRunner;

	delete fPlatformDelegate;
}

// Init
void
TextToolState::Init()
{
	if (!fSelection->IsEmpty())
		ObjectSelected(fSelection->SelectableAt(0), NULL);
	fSelection->AddListener(this);
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

	SetText(NULL);
	DragStateViewState::Cleanup();
	fSelection->RemoveListener(this);
}

// MessageReceived
bool
TextToolState::MessageReceived(BMessage* message, UndoableEdit** _edit)
{
	bool handled = true;

	switch (message->what) {
		case B_COPY:
			Copy(_SelectionStart(), _SelectionLength());
			break;

		case B_PASTE:
			Paste();
			break;

		case MSG_TEXT_CHANGED:
		{
			Text* text;
			if (message->FindPointer("text", (void**)&text) == B_OK
				&& text != NULL && text == fText) {
				SetObjectToCanvasTransformation(fText->Transformation());
				UpdateBounds();
				UpdateDragState();
				_AdoptStyleAtOffset(fCaretOffset - 1);
			}
			break;
		}
		case MSG_CARET_PULSE:
			fShowCaret = !fShowCaret;
			UpdateBounds();
			break;
		default:
			handled = DragStateViewState::MessageReceived(message, _edit);
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
	UndoableEdit** _edit)
{
	if (fText != NULL) {
		*_edit = NULL;

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

	return DragStateViewState::HandleKeyDown(event, _edit);
}

// HandleKeyUp
bool
TextToolState::HandleKeyUp(const StateView::KeyEvent& event,
	UndoableEdit** _edit)
{
	return DragStateViewState::HandleKeyUp(event, _edit);
}

// #pragma mark -

// Draw
void
TextToolState::Draw(PlatformDrawContext& drawContext)
{
	if (fText == NULL)
		return;

	_DrawControls(drawContext);
	if (fSelectionAnchorOffset == fCaretOffset) {
		if (fShowCaret)
			_DrawCaret(drawContext, fCaretOffset);
	} else {
		if (fSelectionAnchorOffset < fCaretOffset)
			_DrawSelection(drawContext, fSelectionAnchorOffset, fCaretOffset);
		else
			_DrawSelection(drawContext, fCaretOffset, fSelectionAnchorOffset);
	}
}

// Bounds
BRect
TextToolState::Bounds() const
{
	if (fText == NULL)
		return BRect(0, 0, -1, -1);

	BRect bounds = fText->Bounds();
	bounds.right = fText->ActualWidth();
	bounds.InsetBy(-15, -15);
	TransformObjectToView(&bounds);
	return bounds;
}

// #pragma mark -

// StartTransaction
UndoableEdit*
TextToolState::StartTransaction(const char* editName)
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

		BPoint widthOffset(fText->ActualWidth(), -10.0f / scaleY / zoomLevel);
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
	
	if (fText != NULL)
		return fDeselectTextState;

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
		BMessage message(MSG_TEXT_CHANGED);
		if (message.AddPointer("text", fText) == B_OK)
			View()->PostMessage(message);
	}
	if (object == fCurrentColor && !fIgnoreColorColorNotifiactions) {
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

	if (!fInsertionLayer->AddObject(text, fInsertionIndex)) {
		fprintf(stderr, "TextToolState::CreateText(): Failed to add "
			"Text to Layer. Out of memory\n");
		text->RemoveReference();
		return false;
	}

	fInsertionIndex++;

	SetText(text, true);
	_SetCaretOffset(initialText.CountChars(), true, true, false);

	// Our reference to this object was transferred to the Layer

	View()->PerformEdit(new(std::nothrow) ObjectAddedEdit(text,
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

	fSelectionAnchorOffset = 0;
	fCaretOffset = 0;
	fCaretAnchorX = 0.0;
	_AdoptStyleAtOffset(fCaretOffset);
	_UpdateConfigViewSelection();
}

// OffsetTextBy
void
TextToolState::OffsetTextBy(BPoint offset)
{
	if (fText != NULL) {
		Transformable transform(*fText);
		// TODO: Not correct...
		transform.TranslateBy(offset);
		View()->PerformEdit(new(std::nothrow) TransformObjectEdit(
			fText, transform));
	}
}

// Insert
void
TextToolState::Insert(int32 textOffset, const char* text,
	bool setCaretOffset)
{
	if (fText != NULL) {
		CompoundEdit* compoundEdit = NULL;

		if (setCaretOffset && _HasSelection()) {
			int32 start = _SelectionStart();
			UndoableEditRef removeEdit(new(std::nothrow) RemoveTextEdit(
				fText, start, _SelectionLength()), true);
			if (removeEdit.Get() == NULL)
				return;

			compoundEdit = new(std::nothrow) CompoundEdit("Insert text");
			if (compoundEdit == NULL)
				return;

			compoundEdit->AppendEdit(removeEdit);

			textOffset = start;
		}

		UndoableEditRef insertEdit(
			new(std::nothrow) InsertTextEdit(
				fText, textOffset, text, Font(fFontFamily, fFontStyle, fSize),
				fGlyphSpacing, fFauxWeight, fFauxItalic, fStyle),
			true);

		if (compoundEdit != NULL) {
			compoundEdit->AppendEdit(insertEdit);
			View()->PerformEdit(compoundEdit);
		} else {
			View()->PerformEdit(insertEdit);
		}

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
		View()->PerformEdit(
			new(std::nothrow) RemoveTextEdit(fText, textOffset, length)
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
		View()->PerformEdit(new(std::nothrow) SetTextStyleEdit(
			fText, _SelectionStart(), _SelectionLength(), family, style));
		UpdateBounds();
	}
}

// SetSize
void
TextToolState::SetSize(float size)
{
	fSize = size;
	if (_HasSelection()) {
		View()->PerformEdit(new(std::nothrow) SetTextStyleEdit(
			fText, _SelectionStart(), _SelectionLength(), size));
		UpdateBounds();
	}
}

// SetTextAlignment
void
TextToolState::SetTextAlignment(uint32 alignment)
{
	fTextAlignment = alignment;
	if (fText != NULL) {
		View()->PerformEdit(new(std::nothrow) SetTextAlignmentEdit(
			fText, alignment));
		UpdateBounds();
	}
}

// SetGlyphSpacing
void
TextToolState::SetGlyphSpacing(double spacing)
{
	fGlyphSpacing = spacing;
	if (_HasSelection()) {
		SetTextStyleEdit* edit = new(std::nothrow) SetTextStyleEdit(
			fText, _SelectionStart(), _SelectionLength());
		if (edit != NULL) {
			edit->SetGlyphSpacing(spacing);
			View()->PerformEdit(edit);
		}
		UpdateBounds();
	}
}

// SetFauxWeight
void
TextToolState::SetFauxWeight(double fauxWeight)
{
	fFauxWeight = fauxWeight;
	if (_HasSelection()) {
		SetTextStyleEdit* edit = new(std::nothrow) SetTextStyleEdit(
			fText, _SelectionStart(), _SelectionLength());
		if (edit != NULL) {
			edit->SetFauxWeight(fauxWeight);
			View()->PerformEdit(edit);
		}
		UpdateBounds();
	}
}

// SetFauxItalic
void
TextToolState::SetFauxItalic(double fauxItalic)
{
	fFauxItalic = fauxItalic;
	if (_HasSelection()) {
		SetTextStyleEdit* edit = new(std::nothrow) SetTextStyleEdit(
			fText, _SelectionStart(), _SelectionLength());
		if (edit != NULL) {
			edit->SetFauxItalic(fauxItalic);
			View()->PerformEdit(edit);
		}
		UpdateBounds();
	}
}

// SetWidth
void
TextToolState::SetWidth(float width)
{
	if (fText != NULL) {
		View()->PerformEdit(new(std::nothrow) SetTextWidthEdit(
			fText, width));
	}
}

// SetColor
void
TextToolState::SetColor(const rgb_color& color)
{
	_SetStyle(color);

	if (_HasSelection()) {
		View()->PerformEdit(new(std::nothrow) SetTextStyleEdit(
			fText, _SelectionStart(), _SelectionLength(), color));
	}
}

// Width
float
TextToolState::Width() const
{
	if (fText != NULL)
		return fText->ActualWidth();
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

// Copy
void
TextToolState::Copy(int32 textOffset, int32 length) const
{
	if (length == 0 || fText == NULL)
		return;

	BClipboard* clipboard = be_clipboard;

	if (clipboard == NULL || !clipboard->Lock())
		return;
	
	clipboard->Clear();

	BMessage* data = clipboard->Data();
	if (data != NULL) {
		BString clip;
		fText->GetText().CopyInto(clip, textOffset, length);
		data->AddData("text/plain", B_MIME_TYPE, clip.String(), clip.Length());
		
		// TODO: Support for "application/x-vnd.Be-text_run_array"
		
		clipboard->Commit();
	}

	clipboard->Unlock();
}

// Paste
void
TextToolState::Paste()
{
	if (fText == NULL)
		return;

	BClipboard* clipboard = be_clipboard;

	if (clipboard == NULL || !clipboard->Lock())
		return;

	BMessage* data = clipboard->Data();
	const char* text = NULL;
	ssize_t length = 0;
	if (data != NULL && data->FindData("text/plain", B_MIME_TYPE,
			(const void**)&text, &length) == B_OK) {
		BString string(text, length);
		Insert(fCaretOffset, string.String());
	}

	clipboard->Unlock();

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
		message.AddDouble("size", fSize);
		message.AddInt32("alignment", fTextAlignment);
		message.AddDouble("glyph spacing", fGlyphSpacing);
		message.AddDouble("faux weight", fFauxWeight);
		message.AddDouble("faux italic", fFauxItalic);
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
	message.AddInt32("anchor", fSelectionAnchorOffset);
	message.AddInt32("caret", fCaretOffset);
	fConfigViewMessenger.SendMessage(&message);
}

// _DrawControls
void
TextToolState::_DrawControls(PlatformDrawContext& drawContext)
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

	BPoint widthOffset(fText->ActualWidth(), -10.0f / scaleY);
	TransformObjectToView(&widthOffset, true);

	fPlatformDelegate->DrawControls(drawContext, origin, widthOffset);
}

// _DrawCaret
void
TextToolState::_DrawCaret(PlatformDrawContext& drawContext, int32 textOffset)
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

	fPlatformDelegate->DrawInvertedShape(drawContext, shape);
}

// _DrawSelection
void
TextToolState::_DrawSelection(PlatformDrawContext& drawContext,
	int32 startOffset, int32 endOffset)
{
	BShape shape;
	_GetSelectionShape(fText->getTextLayout(), shape, startOffset, endOffset);
	fPlatformDelegate->DrawInvertedShape(drawContext, shape);
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
	if (lineIndex < 0) {
		_SetCaretOffset(0, false, select, true);
		return;
	}
	if (lineIndex >= layout.getLineCount()) {
		_SetCaretOffset(layout.getGlyphCount(), false, select, true);
		return;
	}

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
	} else if (startLineIndex == endLineIndex - 1
		&& endX1 <= startX1) {
		// Selection on two lines, with gap:
		// ---------
		// ------###
		// ##-------
		// ---------
		BPoint lt(startX1, startY1);
		BPoint rt(layout.getWidth(), startY1);
		BPoint rb(layout.getWidth(), startY2);
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

		lt = BPoint(0, endY1);
		rt = BPoint(endX1, endY1);
		rb = BPoint(endX1, endY2);
		lb = BPoint(0, endY2);

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
	return abs(fCaretOffset - fSelectionAnchorOffset);
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
	double glyphSpacing;
	double fauxWeight;
	double fauxItalic;
	TextRenderer::Color fgColor;
	bool strikeOut;
	TextRenderer::Color strikeColor;
	bool underline;
	unsigned underlineStyle;
	TextRenderer::Color underlineColor;

	if (!layout.getInfo(textOffset, font, glyphSpacing, fauxWeight, fauxItalic,
			fgColor, strikeOut, strikeColor, underline, underlineStyle,
			underlineColor)) {
		return;
	}

	uint32 alignment = fText->Alignment();

	if (fSize != font.getSize() || fFontFamily != font.getFamily()
		|| fFontStyle != font.getStyle() || alignment != fTextAlignment
		|| fGlyphSpacing != glyphSpacing || fFauxWeight != fauxWeight
		|| fFauxItalic != fauxItalic) {
		fSize = font.getSize();
		fFontFamily = font.getFamily();
		fFontStyle = font.getStyle();
		fTextAlignment = alignment;
		fGlyphSpacing = glyphSpacing;
		fFauxWeight = fauxWeight;
		fFauxItalic = fauxItalic;

		_UpdateConfigView();
	}

	fgColor.demultiply();
	rgb_color color;
	color.red = RenderEngine::LinearToGamma(fgColor.r);
	color.green = RenderEngine::LinearToGamma(fgColor.g);
	color.blue = RenderEngine::LinearToGamma(fgColor.b);
	color.alpha = fgColor.a >> 8;

	fIgnoreColorColorNotifiactions = true;
	fCurrentColor->SetColor(color);
	fIgnoreColorColorNotifiactions = false;

	_SetStyle(color);
}

// _SetStyle
void
TextToolState::_SetStyle(const rgb_color& color)
{
	::Style* style = new(std::nothrow) ::Style();
	if (style == NULL)
		return;

	style->FillPaint()->SetColor(color);
	fStyle.SetTo(style, true);
}

#endif	// _TEXTTOOLSTATE_CPP_
