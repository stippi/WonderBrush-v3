/*
 * Copyright 2009-2012 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef BRUSH_TOOL_STATE_H
#define BRUSH_TOOL_STATE_H

#include "Brush.h"
#include "Selection.h"
#include "TransformViewState.h"

class BrushStroke;
class CurrentColor;
class Document;
class Layer;

class BrushToolState : public TransformViewState,
	public Selection::Controller {
public:
								BrushToolState(StateView* view,
									Document* document, Selection* selection,
									CurrentColor* color, Brush& brush);
	virtual						~BrushToolState();

	// ViewState interface
	virtual	bool				MessageReceived(BMessage* message,
									Command** _command);
	virtual void				MouseDown(const MouseInfo& info);
	virtual void				MouseMoved(const MouseInfo& info);
	virtual Command*			MouseUp();

	virtual void				Draw(PlatformDrawContext& drawContext);

	virtual	BRect				Bounds() const;

	// BrushToolState
			void				SetInsertionInfo(Layer* layer, int32 index);

private:
			void				_AppendPoint(const MouseInfo& info);

			Document*			fDocument;
			Selection*			fSelection;
			CurrentColor*		fCurrentColor;

			Layer*				fInsertionLayer;
			int32				fInsertionIndex;

			Brush&				fBrush;
			BrushStroke*		fBrushStroke;
};

#endif // BRUSH_TOOL_STATE_H
