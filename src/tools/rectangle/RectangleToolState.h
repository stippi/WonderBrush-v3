/*
 * Copyright 2012-2013 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef RECTANGLE_TOOL_STATE_H
#define RECTANGLE_TOOL_STATE_H

#include <Messenger.h>

#include "DragStateViewState.h"
#include "HashSetHugo.h"
#include "List.h"
#include "Selection.h"
#include "Rect.h"
#include "Style.h"

class BMessageRunner;
class BShape;
class CurrentColor;
class Document;
class Layer;

class RectangleToolState : public DragStateViewState,
	public Selection::Controller, public Selection::Listener,
	public Listener {
public:
								RectangleToolState(StateView* view,
									Document* document, Selection* selection,
									CurrentColor* color,
									const BMessenger& configView);
	virtual						~RectangleToolState();

	// ViewState interface
	virtual	void				Init();
	virtual	void				Cleanup();

	virtual	bool				MessageReceived(BMessage* message,
									UndoableEdit** _edit);

	// mouse tracking
	virtual	UndoableEdit*		MouseUp();

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

public:
	// RectangleToolState
			void				SetInsertionInfo(Layer* layer, int32 index);
			bool				CreateRectangle(BPoint canvasLocation);

			void				SetRectangle(Rect* rectangle,
									bool modifySelection = false);

private:
			void				_DrawControls(PlatformDrawContext& drawContext);

			void				_AdoptRectanglePaint();

private:
			class PlatformDelegate;

			class CreateRectangleState;
			
			friend class CreateRectangleState;

private:
			PlatformDelegate*	fPlatformDelegate;

			CreateRectangleState*	fCreateRectangleState;

			Document*			fDocument;
			Selection*			fSelection;
			CurrentColor*		fCurrentColor;

			BMessenger			fConfigViewMessenger;

			Layer*				fInsertionLayer;
			int32				fInsertionIndex;

			Rect*				fRectangle;

			bool				fIgnoreColorNotifiactions;
};

#endif // RECTANGLE_TOOL_STATE_H