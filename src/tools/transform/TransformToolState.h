/*
 * Copyright 2009-2010 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef TRANSFORM_TOOL_STATE_H
#define TRANSFORM_TOOL_STATE_H

#include <Messenger.h>

#include "AbstractLOAdapter.h"
#include "DragStateViewState.h"
#include "Rect.h"
#include "Selection.h"
#include "Shape.h"

class Document;
class Layer;
class Object;

enum {
	MSG_TRANSFORMATION_CHANGED	= 'trch',
};

class TransformToolState : public DragStateViewState,
	public Selection::Controller, public Selection::Listener,
	public Listener {
public:
								TransformToolState(StateView* view,
									const BRect& box, Document* document,
									Selection* selection,
									const BMessenger& configView);
	virtual						~TransformToolState();

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

	// Listener interface
	virtual	void				ObjectChanged(const Notifier* object);

	// TransformToolState
			void				SetObject(Object* object,
									bool modifySelection = false);
			void				AdoptObject();
			void				SetBox(const BRect& box);
			void				SetModifiedBox(const BRect& box,
									bool update = true);
	inline	const BRect&		Box() const
									{ return fOriginalBox; }
	inline	const BRect&		ModifiedBox() const
									{ return fModifiedBox; }

			void				SetPivot(const BPoint& pivot,
									bool update = true);
	inline	const BPoint&		Pivot() const
									{ return fPivot; }

	inline	float				TranslationX() const;
	inline	float				TranslationY() const;
	inline	BPoint				Translation() const;
			void				SetLocalRotation(double rotation);
	inline	double				LocalRotation() const
									{ return fRotation; }
	inline	double				LocalXScale() const;
	inline	double				LocalYScale() const;

	inline	Transformable		UpdateAdditionalTransformation();

private:
			BRect				fOriginalBox;
			BRect				fModifiedBox;
			BPoint				fPivot;
			double				fRotation;

			BMessenger			fConfigViewMessenger;

private:
			class PickObjectState;
			class DragPivotState;
			class DragBoxState;
			class DragCornerState;
			class DragSideState;
			class DragRotationState;

			friend class PickObjectState;

			PickObjectState*	fPickObjectState;

			DragPivotState*		fDragPivotState;
			DragBoxState*		fDragBoxState;
			DragRotationState*	fDragRotationState;

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

			Object*				fObject;
			Transformable		fOriginalTransformation;
			Transformable		fParentGlobalTransformation;

			bool				fIgnoreObjectEvents;
};

#endif // TRANSFORM_TOOL_STATE_H
