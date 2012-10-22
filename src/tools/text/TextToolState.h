/*
 * Copyright 2012 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef TEXT_TOOL_STATE_H
#define TEXT_TOOL_STATE_H

#include "Selection.h"
#include "TransformViewState.h"

class Document;
class Layer;
class Text;

enum {
	MSG_LAYOUT_CHANGED			= 'lych',
};

class TextToolState : public TransformViewState,
	public Selection::Controller {
public:
								TextToolState(StateView* view,
									Document* document, Selection* selection);
	virtual						~TextToolState();

	// ViewState interface
	virtual	bool				MessageReceived(BMessage* message,
									Command** _command);
	virtual void				MouseDown(const MouseInfo& info);
	virtual void				MouseMoved(const MouseInfo& info);
	virtual Command*			MouseUp();

	virtual void				Draw(BView* view, BRect updateRect);

	virtual	BRect				Bounds() const;

	// TextToolState
			void				SetInsertionInfo(Layer* layer, int32 index);

private:
			Document*			fDocument;
			Selection*			fSelection;

			Layer*				fInsertionLayer;
			int32				fInsertionIndex;

			Text*				fText;
};

#endif // TEXT_TOOL_STATE_H
