/*
 * Copyright 2012 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef TEXT_TOOL_STATE_H
#define TEXT_TOOL_STATE_H

#include <Messenger.h>

#include "DragStateViewState.h"
#include "Selection.h"
#include "Style.h"

class BMessageRunner;
class Document;
class Layer;
class Text;
class TextLayout;

enum {
	MSG_LAYOUT_CHANGED			= 'lych',
};

class TextToolState : public DragStateViewState,
	public Selection::Controller, public Selection::Listener,
	public Listener {
public:
								TextToolState(StateView* view,
									Document* document, Selection* selection,
									const BMessenger& configView);
	virtual						~TextToolState();

	// ViewState interface
	virtual	void				Init();
	virtual	void				Cleanup();

	virtual	bool				MessageReceived(BMessage* message,
									Command** _command);

	// modifiers
	virtual	void				ModifiersChanged(uint32 modifiers);

	// TODO: mouse wheel
	virtual	bool				HandleKeyDown(const StateView::KeyEvent& event,
									Command** _command);
	virtual	bool				HandleKeyUp(const StateView::KeyEvent& event,
									Command** _command);

	virtual void				Draw(BView* view, BRect updateRect);

	virtual	BRect				Bounds() const;

	// DragStateViewState interface
	virtual	Command*			StartTransaction(const char* commandName);

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
			void				SetSize(float size);
			void				SetSize(float size, int32 textOffset,
									int32 length);
			
			void				SetWidth(float width);
			float				Width() const;

			void				SelectionChanged(int32 startOffset,
									int32 endOffset);

			void				SetCaretAnchor(const BPoint& location);

private:
			void				_UpdateConfigView() const;
			
			void				_DrawControls(BView* view);
			void				_DrawCaret(BView* view, int32 textOffset);

			void				_LineStart();
			void				_LineEnd();

			void				_LineUp();
			void				_LineDown();
			void				_MoveToLine(TextLayout& layout,
									int32 lineIndex);

			void				_SetCaretOffset(int32 offset,
									bool updateAnchor);

private:
			class PickTextState;
			class CreateTextState;
			class DragLeftTopState;
			class DragWidthState;
			class DragCaretState;

			friend class PickTextState;

			PickTextState*		fPickTextState;
			CreateTextState*	fCreateTextState;
			DragLeftTopState*	fDragLeftTopState;
			DragWidthState*		fDragWidthState;
			DragCaretState*		fDragCaretState;

			Document*			fDocument;
			Selection*			fSelection;

			BMessenger			fConfigViewMessenger;

			Layer*				fInsertionLayer;
			int32				fInsertionIndex;

			Text*				fText;

			BString				fNextText;

			int32				fCaretOffset;
			bool				fShowCaret;
			double				fCaretAnchorX;
			BMessageRunner*		fCaretPulseRunner;

			StyleRef			fStyle;
			double				fSize;
};

#endif // TEXT_TOOL_STATE_H
