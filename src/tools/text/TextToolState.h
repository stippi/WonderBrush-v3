/*
 * Copyright 2012-2013 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef TEXT_TOOL_STATE_H
#define TEXT_TOOL_STATE_H

#include <Messenger.h>

#include "DragStateViewState.h"
#include "Selection.h"
#include "Style.h"

class BMessageRunner;
class BShape;
class CurrentColor;
class Document;
class Layer;
class Text;
class TextLayout;

enum {
	MSG_LAYOUT_CHANGED			= 'lych',
	MSG_SET_SELECTION			= 'stsl',
};

class TextToolState : public DragStateViewState,
	public Selection::Controller, public Selection::Listener,
	public Listener {
public:
								TextToolState(StateView* view,
									Document* document, Selection* selection,
									CurrentColor* color,
									const BMessenger& configView);
	virtual						~TextToolState();

	// ViewState interface
	virtual	void				Init();
	virtual	void				Cleanup();

	virtual	bool				MessageReceived(BMessage* message,
									UndoableEdit** _edit);

	// modifiers
	virtual	void				ModifiersChanged(uint32 modifiers);

	// TODO: mouse wheel
	virtual	bool				HandleKeyDown(const StateView::KeyEvent& event,
									UndoableEdit** _edit);
	virtual	bool				HandleKeyUp(const StateView::KeyEvent& event,
									UndoableEdit** _edit);

	virtual void				Draw(PlatformDrawContext& drawContext);

	virtual	BRect				Bounds() const;

	// DragStateViewState interface
	virtual	UndoableEdit*		StartTransaction(const char* editName);

	virtual	DragState*			DragStateFor(BPoint canvasWhere,
									float zoomLevel) const;

	// Selection::Listener interface
	virtual	void				ObjectSelected(const Selectable& object,
									const Selection::Controller* controller);
	virtual	void				ObjectDeselected(const Selectable& object,
									const Selection::Controller* controller);

	// Listener interface
	virtual	void				ObjectChanged(const Notifier* object);

	// TextToolState
			void				SetInsertionInfo(Layer* layer, int32 index);
			bool				CreateText(BPoint canvasLocation);

			void				SetText(Text* text,
									bool modifySelection = false);

			void				OffsetTextBy(BPoint offset);
			void				Insert(int32 textOffset, const char* text,
									bool setCaretOffset = true);
			void				Remove(int32 textOffset, int32 length,
									bool setCaretOffset = true);

			void				SetFont(const char* family, const char* style);
			void				SetSize(float size);

			void				SetTextAlignment(uint32 alignment);

			void				SetGlyphSpacing(double spacing);

			void				SetWidth(float width);
			float				Width() const;

			void				SetColor(const rgb_color& color);

			void				SelectionChanged(int32 startOffset,
									int32 endOffset);

			void				SetCaret(const BPoint& location, bool select);

private:
			void				_UpdateConfigView() const;
			void				_UpdateConfigViewSelection() const;

			void				_DrawControls(PlatformDrawContext& drawContext);

			void				_DrawCaret(PlatformDrawContext& drawContext,
									int32 textOffset);
			void				_DrawSelection(PlatformDrawContext& drawContext,
									int32 startOffset, int32 endOffset);

			void				_LineStart(bool select);
			void				_LineEnd(bool select);

			void				_LineUp(bool select);
			void				_LineDown(bool select);
			void				_MoveToLine(TextLayout& layout,
									int32 lineIndex, bool select);

			void				_SetCaretOffset(int32 offset,
									bool updateAnchor,
									bool lockSelectionAnchor,
									bool updateSelectionStyle);

			void				_GetSelectionShape(TextLayout& layout,
									BShape& shape, int32 start,
									int32 end) const;

			bool				_HasSelection() const;
			int32				_SelectionStart() const;
			int32				_SelectionEnd() const;
			int32				_SelectionLength() const;

			void				_AdoptStyleAtOffset(int32 textOffset);
			void				_SetStyle(const rgb_color& color);

private:
			class PlatformDelegate;

			class PickTextState;
			class CreateTextState;
			class DragLeftTopState;
			class DragWidthState;
			class DragCaretState;

			friend class PickTextState;

			PlatformDelegate*	fPlatformDelegate;

			PickTextState*		fPickTextState;
			CreateTextState*	fCreateTextState;
			DragLeftTopState*	fDragLeftTopState;
			DragWidthState*		fDragWidthState;
			DragCaretState*		fDragCaretState;

			Document*			fDocument;
			Selection*			fSelection;
			CurrentColor*		fCurrentColor;

			BMessenger			fConfigViewMessenger;

			Layer*				fInsertionLayer;
			int32				fInsertionIndex;

			Text*				fText;

			BString				fNextText;

			int32				fSelectionAnchorOffset;
			int32				fCaretOffset;
			bool				fShowCaret;
			double				fCaretAnchorX;
			BMessageRunner*		fCaretPulseRunner;

			StyleRef			fStyle;
			BString				fFontFamily;
			BString				fFontStyle;
			double				fSize;
			uint32				fTextAlignment;
			double				fGlyphSpacing;

			bool				fIgnoreColorColorNotifiactions;
};

#endif // TEXT_TOOL_STATE_H
