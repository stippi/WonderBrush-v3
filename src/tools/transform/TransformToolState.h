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

	virtual void				Draw(PlatformDrawContext& drawContext);

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
			class PlatformDelegate;
			struct DrawParameters;

			class PickObjectState;
			class DragPivotState;
			class DragBoxState;
			class DragCornerState;
			class DragSideState;
			class DragRotationState;

			friend class PickObjectState;

private:
			PlatformDelegate*	fPlatformDelegate;

			BRect				fOriginalBox;
			BRect				fModifiedBox;
			BPoint				fPivot;
			double				fRotation;

			BMessenger			fConfigViewMessenger;

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


struct TransformToolState::DrawParameters {
	float insetX;
	float insetY;
	float insetFill1X;
	float insetFill1Y;
	float insetFill2X;
	float insetFill2Y;

	bool subpixelPrecise;

	BPoint lt;
	BPoint rt;
	BPoint rb;
	BPoint lb;

	BPoint lt1;
	BPoint lt2;
	BPoint lt3;

	BPoint rt1;
	BPoint rt2;
	BPoint rt3;

	BPoint rb1;
	BPoint rb2;
	BPoint rb3;

	BPoint lb1;
	BPoint lb2;
	BPoint lb3;

	BPoint ltF0;
	BPoint ltF1;
	BPoint ltF2;
	BPoint ltF3;

	BPoint rtF0;
	BPoint rtF1;
	BPoint rtF2;
	BPoint rtF3;

	BPoint rbF0;
	BPoint rbF1;
	BPoint rbF2;
	BPoint rbF3;

	BPoint lbF0;
	BPoint lbF1;
	BPoint lbF2;
	BPoint lbF3;

	BPoint pivot;

	static const float pivotSize = 3;

	DrawParameters(TransformToolState* state, const BRect & modifiedBox,
		float scaleX, float scaleY, const BPoint & pivot, bool subpixelPrecise)
		:
		insetX(6 / scaleX),
		insetY(6 / scaleY),
		insetFill1X(5.2 / scaleX),
		insetFill1Y(5.2 / scaleY),
		insetFill2X(0.8 / scaleX),
		insetFill2Y(0.8 / scaleY),

		subpixelPrecise(subpixelPrecise),

		lt(min_c(modifiedBox.left, modifiedBox.right),
			min_c(modifiedBox.top, modifiedBox.bottom)),
		rt(max_c(modifiedBox.left, modifiedBox.right),
			min_c(modifiedBox.top, modifiedBox.bottom)),
		rb(max_c(modifiedBox.left, modifiedBox.right),
			max_c(modifiedBox.top, modifiedBox.bottom)),
		lb(min_c(modifiedBox.left, modifiedBox.right),
			max_c(modifiedBox.top, modifiedBox.bottom)),

		lt1(lt.x - insetX, lt.y),
		lt2(lt.x - insetX, lt.y - insetY),
		lt3(lt.x, lt.y - insetY),

		rt1(rt.x, rt.y - insetY),
		rt2(rt.x + insetX, rt.y - insetY),
		rt3(rt.x + insetX, rt.y),

		rb1(rb.x + insetX, rb.y),
		rb2(rb.x + insetX, rb.y + insetY),
		rb3(rb.x, rb.y + insetY),

		lb1(lb.x, lb.y + insetY),
		lb2(lb.x - insetX, lb.y + insetY),
		lb3(lb.x - insetX, lb.y),

		ltF0(lt.x - insetFill2X, lt.y - insetFill2Y),
		ltF1(lt.x - insetFill1X, lt.y - insetFill2Y),
		ltF2(lt.x - insetFill1X, lt.y - insetFill1Y),
		ltF3(lt.x - insetFill2X, lt.y - insetFill1Y),

		rtF0(rt.x + insetFill2X, rt.y - insetFill2Y),
		rtF1(rt.x + insetFill2X, rt.y - insetFill1Y),
		rtF2(rt.x + insetFill1X, rt.y - insetFill1Y),
		rtF3(rt.x + insetFill1X, rt.y - insetFill2Y),

		rbF0(rb.x + insetFill2X, rb.y + insetFill2Y),
		rbF1(rb.x + insetFill1X, rb.y + insetFill2Y),
		rbF2(rb.x + insetFill1X, rb.y + insetFill1Y),
		rbF3(rb.x + insetFill2X, rb.y + insetFill1Y),

		lbF0(lb.x - insetFill2X, lb.y + insetFill2Y),
		lbF1(lb.x - insetFill2X, lb.y + insetFill1Y),
		lbF2(lb.x - insetFill1X, lb.y + insetFill1Y),
		lbF3(lb.x - insetFill1X, lb.y + insetFill2Y),

		pivot(pivot)
	{
		bool round = !subpixelPrecise;

		state->TransformObjectToView(&lt, round);
		state->TransformObjectToView(&rt, round);
		state->TransformObjectToView(&rb, round);
		state->TransformObjectToView(&lb, round);

		state->TransformObjectToView(&lt1, round);
		state->TransformObjectToView(&lt2, round);
		state->TransformObjectToView(&lt3, round);

		state->TransformObjectToView(&rt1, round);
		state->TransformObjectToView(&rt2, round);
		state->TransformObjectToView(&rt3, round);

		state->TransformObjectToView(&rb1, round);
		state->TransformObjectToView(&rb2, round);
		state->TransformObjectToView(&rb3, round);

		state->TransformObjectToView(&lb1, round);
		state->TransformObjectToView(&lb2, round);
		state->TransformObjectToView(&lb3, round);

		state->TransformObjectToView(&ltF0, round);
		state->TransformObjectToView(&ltF1, round);
		state->TransformObjectToView(&ltF2, round);
		state->TransformObjectToView(&ltF3, round);

		state->TransformObjectToView(&rtF0, round);
		state->TransformObjectToView(&rtF1, round);
		state->TransformObjectToView(&rtF2, round);
		state->TransformObjectToView(&rtF3, round);

		state->TransformObjectToView(&rbF0, round);
		state->TransformObjectToView(&rbF1, round);
		state->TransformObjectToView(&rbF2, round);
		state->TransformObjectToView(&rbF3, round);

		state->TransformObjectToView(&lbF0, round);
		state->TransformObjectToView(&lbF1, round);
		state->TransformObjectToView(&lbF2, round);
		state->TransformObjectToView(&lbF3, round);

		state->TransformCanvasToView(&this->pivot);
	}
};


#endif // TRANSFORM_TOOL_STATE_H
