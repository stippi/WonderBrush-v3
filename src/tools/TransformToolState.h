/*
 * Copyright 2009-2010 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef TRANSFORM_TOOL_STATE_H
#define TRANSFORM_TOOL_STATE_H

#include "DragStateViewState.h"
#include "Selection.h"

class Document;
class Layer;
class Object;

class TransformToolState : public DragStateViewState,
	public Selection::Controller, public Selection::Listener {
public:
								TransformToolState(StateView* view,
									const BRect& box, Document* document,
									Selection* selection);
	virtual						~TransformToolState();

	// ViewState interface
	virtual void				MouseDown(BPoint where, uint32 buttons,
									uint32 clicks);
	virtual void				MouseMoved(BPoint where, uint32 transit,
									const BMessage* dragMessage);
	virtual Command*			MouseUp();

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

	// TransformToolState
			void				SetObject(Object* object);
			void				SetTransformable(Transformable* object);
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
			class PickObjectState;
			class DragBoxState;
			class DragCornerState;
			class DragSideState;

			friend class PickObjectState;

			PickObjectState*	fPickObjectState;

			DragBoxState*		fDragBoxState;

			DragCornerState*	fDragLTState;
			DragCornerState*	fDragRTState;
			DragCornerState*	fDragRBState;
			DragCornerState*	fDragLBState;

			DragSideState*		fDragLState;
			DragSideState*		fDragTState;
			DragSideState*		fDragRState;
			DragSideState*		fDragBState;

			Document*			fDocument;
			Selection*			fSelection;

			Transformable*		fObject;
			Transformable		fOriginalTransformation;
};

#endif // TRANSFORM_TOOL_STATE_H
