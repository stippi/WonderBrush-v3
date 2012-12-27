/*
 * Copyright 2012 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef PATH_TOOL_STATE_H
#define PATH_TOOL_STATE_H

#include <Messenger.h>

#include "DragStateViewState.h"
#include "Selection.h"
#include "Style.h"

class BMessageRunner;
class BShape;
class CurrentColor;
class Document;
class Layer;
class Shape;

class PathToolState : public DragStateViewState,
	public Selection::Controller, public Selection::Listener,
	public Listener {
public:
								PathToolState(StateView* view,
									Document* document, Selection* selection,
									CurrentColor* color,
									const BMessenger& configView);
	virtual						~PathToolState();

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

	// PathToolState
			void				SetInsertionInfo(Layer* layer, int32 index);
			bool				CreateShape(BPoint canvasLocation);

			void				SetShape(Shape* shape,
									bool modifySelection = false);
private:
			void				_DrawControls(PlatformDrawContext& drawContext);

private:
			class PlatformDelegate;

			class PickShapeState;
			class CreateShapeState;

			friend class PickShapeState;

private:
			PlatformDelegate*	fPlatformDelegate;

			PickShapeState*		fPickShapeState;
			CreateShapeState*	fCreateShapeState;

			Document*			fDocument;
			Selection*			fSelection;
			CurrentColor*		fCurrentColor;

			BMessenger			fConfigViewMessenger;

			Layer*				fInsertionLayer;
			int32				fInsertionIndex;

			Shape*				fShape;

			StyleRef			fStyle;

			bool				fIgnoreColorColorNotifiactions;
};

#endif // PATH_TOOL_STATE_H
