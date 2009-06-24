/*
 * Copyright 2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef DRAG_STATE_VIEW_STATE_H
#define DRAG_STATE_VIEW_STATE_H

#include "TransformViewState.h"

class Command;

class DragStateViewState : public TransformViewState {
public:

	class DragState {
	public:
								DragState(DragStateViewState* parent);
		virtual					~DragState();

		virtual	void			SetOrigin(BPoint origin);
		virtual	void			DragTo(BPoint current, uint32 modifiers) = 0;
		virtual	BCursor			ViewCursor(BPoint current) const = 0;

		virtual	const char*		CommandName() const = 0;

	protected:
				BPoint			fOrigin;
				DragStateViewState* fParent;
	};

public:
								DragStateViewState(StateView* view);
	virtual						~DragStateViewState();

	// ViewState interface
	virtual void				MouseDown(BPoint where, uint32 buttons,
									uint32 clicks);
	virtual void				MouseMoved(BPoint where, uint32 transit,
									const BMessage* dragMessage);
	virtual Command*			MouseUp();

	virtual	bool				UpdateCursor();

	// DragStateViewState
	virtual	Command*			StartTransaction(const char* commandName);
	virtual	Command*			FinishTransaction(Command* command);

	virtual	DragState*			DragStateFor(BPoint canvasWhere,
									float zoomLevel) const = 0;

			void				SetDragState(DragState* state);

private:
			DragState*			fCurrentState;
			bool				fDragging;
			Command*			fCurrentCommand;
};

#endif // DRAG_STATE_VIEW_STATE_H
