/*
 * Copyright 2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef TRANSFORM_TOOL_STATE_H
#define TRANSFORM_TOOL_STATE_H

#include "DragStateViewState.h"

class TransformToolState : public DragStateViewState {
public:
								TransformToolState(StateView* view,
									const BRect& box);
	virtual						~TransformToolState();

	// ViewState interface
	virtual void				Draw(BView* view, BRect updateRect);

	virtual	BRect				Bounds() const;

	// DragStateViewState interface
	virtual	Command*			StartTransaction(const char* commandName);

	virtual	DragState*			DragStateFor(BPoint canvasWhere,
									float zoomLevel) const;

			void				SetBox(const BRect& box);
			void				SetModifiedBox(const BRect& box);
	inline	const BRect&		Box() const
									{ return fOriginalBox; }
	inline	const BRect&		ModifiedBox() const
									{ return fModifiedBox; }

			float				LocalXScale() const;
			float				LocalYScale() const;

private:
			BRect				fOriginalBox;
			BRect				fModifiedBox;

private:
	class DragBoxState;
	class DragCornerState;
	class DragSideState;

			DragBoxState*		fDragBoxState;

			DragCornerState*	fDragLTState;
			DragCornerState*	fDragRTState;
			DragCornerState*	fDragRBState;
			DragCornerState*	fDragLBState;

			DragSideState*		fDragLState;
			DragSideState*		fDragTState;
			DragSideState*		fDragRState;
			DragSideState*		fDragBState;
};

#endif // TRANSFORM_TOOL_STATE_H
