/*
 * Copyright 2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef DRAG_STATE_VIEW_STATE_H
#define DRAG_STATE_VIEW_STATE_H

#include "TransformViewState.h"

class UndoableEdit;

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
	virtual void				MouseDown(const MouseInfo& info);
	virtual void				MouseMoved(const MouseInfo& info);
	virtual UndoableEdit*			MouseUp();

	virtual	bool				UpdateCursor();

	// DragStateViewState
	virtual	UndoableEdit*			StartTransaction(const char* commandName);
	virtual	UndoableEdit*			FinishTransaction(UndoableEdit* edit);

	virtual	DragState*			DragStateFor(BPoint canvasWhere,
									float zoomLevel) const = 0;

			void				SetDragState(DragState* state);

			bool				IsDragging() const
									{ return fDragging; }

			void				UpdateDragState();

private:
			DragState*			fCurrentState;
			bool				fDragging;
			UndoableEdit*			fCurrentCommand;
};

#endif // DRAG_STATE_VIEW_STATE_H
