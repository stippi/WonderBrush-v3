/*
 * Copyright 2012 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef TEXT_TOOL_STATE_H
#define TEXT_TOOL_STATE_H

#include <Messenger.h>

#include "Selection.h"
#include "TransformViewState.h"

class Document;
class Layer;
class Text;

enum {
	MSG_LAYOUT_CHANGED			= 'lych',
};

class TextToolState : public TransformViewState,
	public Selection::Controller, public Selection::Listener {
public:
								TextToolState(StateView* view,
									Document* document, Selection* selection,
									const BMessenger& configView);
	virtual						~TextToolState();

	// ViewState interface
	virtual	bool				MessageReceived(BMessage* message,
									Command** _command);
	virtual void				MouseDown(const MouseInfo& info);
	virtual void				MouseMoved(const MouseInfo& info);
	virtual Command*			MouseUp();

	virtual void				Draw(BView* view, BRect updateRect);

	virtual	BRect				Bounds() const;

	// Selection::Listener interface
	virtual	void				ObjectSelected(const Selectable& object,
									const Selection::Controller* controller);
	virtual	void				ObjectDeselected(const Selectable& object,
									const Selection::Controller* controller);

	// TextToolState
			void				SetInsertionInfo(Layer* layer, int32 index);

			void				SetText(Text* text,
									bool modifySelection = false);
private:
			void				_UpdateConfigView() const;

private:
			Document*			fDocument;
			Selection*			fSelection;

			BMessenger			fConfigViewMessenger;

			Layer*				fInsertionLayer;
			int32				fInsertionIndex;

			Text*				fText;
};

#endif // TEXT_TOOL_STATE_H
