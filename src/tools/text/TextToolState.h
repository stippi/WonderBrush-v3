/*
 * Copyright 2012 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef TEXT_TOOL_STATE_H
#define TEXT_TOOL_STATE_H

#include <Messenger.h>

#include "DragStateViewState.h"
#include "Selection.h"

class Document;
class Layer;
class Text;

enum {
	MSG_LAYOUT_CHANGED			= 'lych',
};

class TextToolState : public DragStateViewState,
	public Selection::Controller, public Selection::Listener {
public:
								TextToolState(StateView* view,
									Document* document, Selection* selection,
									const BMessenger& configView);
	virtual						~TextToolState();

	// ViewState interface
	virtual	bool				MessageReceived(BMessage* message,
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

	// TextToolState
			void				SetInsertionInfo(Layer* layer, int32 index);
			bool				CreateText(BPoint canvasLocation);

			void				SetText(Text* text,
									bool modifySelection = false);

			void				OffsetTextBy(BPoint offset);
			void				SetString(const char* text);
			void				SetSize(float size);

private:
			void				_UpdateConfigView() const;

private:
			class PickTextState;
			class CreateTextState;
			class DragLeftTopState;
			class DragWidthState;

			friend class PickTextState;

			PickTextState*		fPickTextState;
			CreateTextState*	fCreateTextState;
			DragLeftTopState*	fDragLeftTopState;
			DragWidthState*		fDragWidthState;

			Document*			fDocument;
			Selection*			fSelection;

			BMessenger			fConfigViewMessenger;

			Layer*				fInsertionLayer;
			int32				fInsertionIndex;

			Text*				fText;
};

#endif // TEXT_TOOL_STATE_H