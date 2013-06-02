/*
 * Copyright 2009-2012 Stephan Aßmus <superstippi@gmx.de>
 * Copyright 2012 Ingo Weinhold <ingo_weinhold@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "TransformToolState.h"
#include "TransformToolStatePlatformDelegate.h"

#include <Cursor.h>

#include <new>

#include <agg_math.h>

#include "EditManager.h"
#include "cursors.h"
#include "Document.h"
#include "Layer.h"
#include "Rect.h"
#include "TransformObjectEdit.h"
#include "Shape.h"
#include "support.h"

enum {
	MSG_ADOPT_OBJECT	= 'ttao',
};

class TransformToolState::DragPivotState
	: public DragStateViewState::DragState {
public:
	DragPivotState(TransformToolState* parent)
		: DragState(parent)
		, fParent(parent)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		DragState::SetOrigin(origin);
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		BPoint offset = current - fOrigin;
		fOrigin = current;

		fParent->SetPivot(fParent->Pivot() + offset);
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		return BCursor(B_CURSOR_ID_FOLLOW_LINK);
	}

	virtual const char* CommandName() const
	{
		return "Drag Pivot";
	}

private:
	TransformToolState*	fParent;
};


class TransformToolState::DragBoxState : public DragStateViewState::DragState {
public:
	DragBoxState(TransformToolState* parent)
		: DragState(parent)
		, fParent(parent)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		fDragStart = origin;
		fStartPivot = fParent->Pivot();

		fParent->TransformCanvasToObject(&origin);
		DragState::SetOrigin(origin);
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		BPoint pivotOffset = current - fDragStart;

		BPoint objectCurrent = current;
		fParent->TransformCanvasToObject(&objectCurrent);
		BPoint boxOffset = objectCurrent - fOrigin;

		BRect box = fParent->ModifiedBox();
		box.OffsetBy(boxOffset);
		fParent->SetModifiedBox(box, false);

		fParent->SetPivot(fStartPivot + pivotOffset);

		fOrigin = current;
		fParent->TransformCanvasToObject(&fOrigin);
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		return BCursor(B_CURSOR_ID_MOVE);
	}

	virtual const char* CommandName() const
	{
		return "Move";
	}

private:
	TransformToolState*	fParent;
	BPoint				fStartPivot;
	BPoint				fDragStart;
};


class TransformToolState::DragCornerState
	: public DragStateViewState::DragState {
public:
	typedef enum {
		LEFT_TOP = 0,
		RIGHT_TOP,
		RIGHT_BOTTOM,
		LEFT_BOTTOM
	} Corner;

	DragCornerState(TransformToolState* parent, Corner corner)
		: DragState(parent)
		, fParent(parent)
		, fCorner(corner)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		BPoint clickTarget;
		switch (fCorner) {
			case LEFT_TOP:
				clickTarget = fParent->ModifiedBox().LeftTop();
				break;
			case RIGHT_TOP:
				clickTarget = fParent->ModifiedBox().RightTop();
				break;
			case RIGHT_BOTTOM:
				clickTarget = fParent->ModifiedBox().RightBottom();
				break;
			case LEFT_BOTTOM:
				clickTarget = fParent->ModifiedBox().LeftBottom();
				break;
		}
		fParent->TransformCanvasToObject(&origin);
		fClickOffset = origin - clickTarget;
		DragState::SetOrigin(origin);
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		fParent->TransformCanvasToObject(&current);
		current -= fClickOffset;
		BRect box = fParent->ModifiedBox();
		switch (fCorner) {
			case LEFT_TOP:
				box.left = current.x;
				box.top = current.y;
				break;
			case RIGHT_TOP:
				box.right = current.x;
				box.top = current.y;
				break;
			case RIGHT_BOTTOM:
				box.right = current.x;
				box.bottom = current.y;
				break;
			case LEFT_BOTTOM:
				box.left = current.x;
				box.bottom = current.y;
				break;
		}
		fParent->SetModifiedBox(box);
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		float rotation = fmod(360.0 - fParent->ViewspaceRotation() + 22.5,
			180.0);
		bool flipX = fParent->LocalXScale() < 0.0;
		bool flipY = fParent->LocalYScale() < 0.0;

		BCursorID cursorID = B_CURSOR_ID_MOVE;
		if (rotation < 45.0) {
			switch (fCorner) {
				case LEFT_TOP:
				case RIGHT_BOTTOM:
					if (flipX) {
						cursorID = flipY
							? B_CURSOR_ID_RESIZE_NORTH_WEST_SOUTH_EAST
							: B_CURSOR_ID_RESIZE_NORTH_EAST_SOUTH_WEST;
					} else {
						cursorID = flipY
							? B_CURSOR_ID_RESIZE_NORTH_EAST_SOUTH_WEST
							: B_CURSOR_ID_RESIZE_NORTH_WEST_SOUTH_EAST;
					}
					break;
				case RIGHT_TOP:
				case LEFT_BOTTOM:
					if (flipX) {
						cursorID = flipY
							? B_CURSOR_ID_RESIZE_NORTH_EAST_SOUTH_WEST
							: B_CURSOR_ID_RESIZE_NORTH_WEST_SOUTH_EAST;
					} else {
						cursorID = flipY
							? B_CURSOR_ID_RESIZE_NORTH_WEST_SOUTH_EAST
							: B_CURSOR_ID_RESIZE_NORTH_EAST_SOUTH_WEST;
					}
					break;
			}
		} else if (rotation < 90.0) {
			switch (fCorner) {
				case LEFT_TOP:
				case RIGHT_BOTTOM:
					if (flipX) {
						cursorID = flipY
							? B_CURSOR_ID_RESIZE_EAST_WEST
							: B_CURSOR_ID_RESIZE_NORTH_SOUTH;
					} else {
						cursorID = flipY
							? B_CURSOR_ID_RESIZE_NORTH_SOUTH
							: B_CURSOR_ID_RESIZE_EAST_WEST;
					}
					break;
				case RIGHT_TOP:
				case LEFT_BOTTOM:
					if (flipX) {
						cursorID = flipY
							? B_CURSOR_ID_RESIZE_NORTH_SOUTH
							: B_CURSOR_ID_RESIZE_EAST_WEST;
					} else {
						cursorID = flipY
							? B_CURSOR_ID_RESIZE_EAST_WEST
							: B_CURSOR_ID_RESIZE_NORTH_SOUTH;
					}
					break;
			}
		} else if (rotation < 135.0) {
			switch (fCorner) {
				case LEFT_TOP:
				case RIGHT_BOTTOM:
					if (flipX) {
						cursorID = flipY
							? B_CURSOR_ID_RESIZE_NORTH_EAST_SOUTH_WEST
							: B_CURSOR_ID_RESIZE_NORTH_WEST_SOUTH_EAST;
					} else {
						cursorID = flipY
							? B_CURSOR_ID_RESIZE_NORTH_WEST_SOUTH_EAST
							: B_CURSOR_ID_RESIZE_NORTH_EAST_SOUTH_WEST;
					}
					break;
				case RIGHT_TOP:
				case LEFT_BOTTOM:
					if (flipX) {
						cursorID = flipY
							? B_CURSOR_ID_RESIZE_NORTH_WEST_SOUTH_EAST
							: B_CURSOR_ID_RESIZE_NORTH_EAST_SOUTH_WEST;
					} else {
						cursorID = flipY
							? B_CURSOR_ID_RESIZE_NORTH_EAST_SOUTH_WEST
							: B_CURSOR_ID_RESIZE_NORTH_WEST_SOUTH_EAST;
					}
					break;
			}
		} else {
			switch (fCorner) {
				case LEFT_TOP:
				case RIGHT_BOTTOM:
					if (flipX) {
						cursorID = flipY
							? B_CURSOR_ID_RESIZE_NORTH_SOUTH
							: B_CURSOR_ID_RESIZE_EAST_WEST;
					} else {
						cursorID = flipY
							? B_CURSOR_ID_RESIZE_EAST_WEST
							: B_CURSOR_ID_RESIZE_NORTH_SOUTH;
					}
					break;
				case RIGHT_TOP:
				case LEFT_BOTTOM:
					if (flipX) {
						cursorID = flipY
							? B_CURSOR_ID_RESIZE_EAST_WEST
							: B_CURSOR_ID_RESIZE_NORTH_SOUTH;
					} else {
						cursorID = flipY
							? B_CURSOR_ID_RESIZE_NORTH_SOUTH
							: B_CURSOR_ID_RESIZE_EAST_WEST;
					}
					break;
			}
		}
		return BCursor(cursorID);
	}

	virtual const char* CommandName() const
	{
		return "Drag Corner";
	}

private:
	TransformToolState*	fParent;
	BPoint				fClickOffset;
	Corner				fCorner;
};


class TransformToolState::DragSideState
	: public DragStateViewState::DragState {
public:
	typedef enum {
		LEFT = 0,
		TOP,
		RIGHT,
		BOTTOM
	} Side;

	DragSideState(TransformToolState* parent, Side side)
		: DragState(parent)
		, fParent(parent)
		, fSide(side)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		fParent->TransformCanvasToObject(&origin);
		switch (fSide) {
			case LEFT:
				fClickOffset = origin.x - fParent->ModifiedBox().left;
				break;
			case TOP:
				fClickOffset = origin.y - fParent->ModifiedBox().top;
				break;
			case RIGHT:
				fClickOffset = origin.x - fParent->ModifiedBox().right;
				break;
			case BOTTOM:
				fClickOffset = origin.y - fParent->ModifiedBox().bottom;
				break;
		}
		DragState::SetOrigin(origin);
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		fParent->TransformCanvasToObject(&current);
		BRect box = fParent->ModifiedBox();
		switch (fSide) {
			case LEFT:
				box.left = current.x - fClickOffset;
				break;
			case TOP:
				box.top = current.y - fClickOffset;
				break;
			case RIGHT:
				box.right = current.x - fClickOffset;
				break;
			case BOTTOM:
				box.bottom = current.y - fClickOffset;
				break;
		}
		fParent->SetModifiedBox(box);
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		float rotation = fmod(360.0 - fParent->ViewspaceRotation() + 22.5,
			180.0);

		BCursorID cursorID = B_CURSOR_ID_MOVE;
		if (rotation < 45.0) {
			switch (fSide) {
				case LEFT:
				case RIGHT:
					cursorID = B_CURSOR_ID_RESIZE_EAST_WEST;
					break;
				case TOP:
				case BOTTOM:
					cursorID = B_CURSOR_ID_RESIZE_NORTH_SOUTH;
					break;
			}
		} else if (rotation < 90.0) {
			switch (fSide) {
				case LEFT:
				case RIGHT:
					cursorID = B_CURSOR_ID_RESIZE_NORTH_EAST_SOUTH_WEST;
					break;
				case TOP:
				case BOTTOM:
					cursorID = B_CURSOR_ID_RESIZE_NORTH_WEST_SOUTH_EAST;
					break;
			}
		} else if (rotation < 135.0) {
			switch (fSide) {
				case LEFT:
				case RIGHT:
					cursorID = B_CURSOR_ID_RESIZE_NORTH_SOUTH;
					break;
				case TOP:
				case BOTTOM:
					cursorID = B_CURSOR_ID_RESIZE_EAST_WEST;
					break;
			}
		} else {
			switch (fSide) {
				case LEFT:
				case RIGHT:
					cursorID = B_CURSOR_ID_RESIZE_NORTH_WEST_SOUTH_EAST;
					break;
				case TOP:
				case BOTTOM:
					cursorID = B_CURSOR_ID_RESIZE_NORTH_EAST_SOUTH_WEST;
					break;
			}
		}
		return BCursor(cursorID);
	}

	virtual const char* CommandName() const
	{
		return "Drag Side";
	}

private:
	TransformToolState*	fParent;
	float				fClickOffset;
	Side				fSide;
};


class TransformToolState::DragRotationState
	: public DragStateViewState::DragState {
public:
	DragRotationState(TransformToolState* parent)
		: DragState(parent)
		, fParent(parent)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		DragState::SetOrigin(origin);
		fOldAngle = fParent->LocalRotation();
		fPivot = fParent->Pivot();
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		double angle = calc_angle(fPivot, fOrigin, current);

		if (modifiers & B_SHIFT_KEY) {
			if (angle < 0.0)
				angle -= 22.5;
			else
				angle += 22.5;
			angle = 45.0 * ((int32)angle / 45);
		}

		fParent->SetLocalRotation(fOldAngle + angle);
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		BPoint pivot(fParent->Pivot());
		BPoint from = pivot + BPoint(sin(22.5 * 180.0 / M_PI) * 50.0,
			-cos(22.5 * 180.0 / M_PI) * 50.0);

		double rotation = calc_angle(pivot, from, current) + 180.0;

		if (rotation < 45.0)
			return BCursor(kRotateLCursor);
		else if (rotation < 90.0)
			return BCursor(kRotateLTCursor);
		else if (rotation < 135.0)
			return BCursor(kRotateTCursor);
		else if (rotation < 180.0)
			return BCursor(kRotateRTCursor);
		else if (rotation < 225.0)
			return BCursor(kRotateRCursor);
		else if (rotation < 270.0)
			return BCursor(kRotateRBCursor);
		else if (rotation < 315.0)
			return BCursor(kRotateBCursor);
		else
			return BCursor(kRotateLBCursor);
	}

	virtual const char* CommandName() const
	{
		return "Rotate";
	}

private:
	TransformToolState*	fParent;
	double				fOldAngle;
	BPoint				fPivot;
};


class TransformToolState::PickObjectState
	: public DragStateViewState::DragState {
public:
	PickObjectState(TransformToolState* parent)
		: DragState(parent)
		, fParent(parent)
		, fObject(NULL)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		// Setup tool and switch to drag box state
		fParent->SetObject(fObject, true);

		if (fObject == NULL)
			return;

		fParent->SetDragState(fParent->fDragBoxState);
		fParent->fDragBoxState->SetOrigin(origin);
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		// Never reached.
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		if (fObject != NULL)
			return BCursor(B_CURSOR_ID_FOLLOW_LINK);
		return BCursor(B_CURSOR_ID_SYSTEM_DEFAULT);
	}

	virtual const char* CommandName() const
	{
		return "Pick object";
	}

	void SetObject(Object* object)
	{
		fObject = object;
	}

private:
	TransformToolState*	fParent;
	Object*				fObject;
};


// #pragma mark -


// constructor
TransformToolState::TransformToolState(StateView* view, const BRect& box,
		Document* document, Selection* selection, const BMessenger& configView)
	: DragStateViewState(view)
	, fPlatformDelegate(new PlatformDelegate(this))
	, fOriginalBox(box)
	, fModifiedBox(box)
//	, fTransformation()
	, fConfigViewMessenger(configView)

	, fPickObjectState(new PickObjectState(this))

	, fDragPivotState(new (std::nothrow) DragPivotState(this))
	, fDragBoxState(new (std::nothrow) DragBoxState(this))
	, fDragRotationState(new (std::nothrow) DragRotationState(this))

	, fDragLTState(new (std::nothrow) DragCornerState(this,
		DragCornerState::LEFT_TOP))
	, fDragRTState(new (std::nothrow) DragCornerState(this,
		DragCornerState::RIGHT_TOP))
	, fDragRBState(new (std::nothrow) DragCornerState(this,
		DragCornerState::RIGHT_BOTTOM))
	, fDragLBState(new (std::nothrow) DragCornerState(this,
		DragCornerState::LEFT_BOTTOM))

	, fDragLState(new (std::nothrow) DragSideState(this,
		DragSideState::LEFT))
	, fDragTState(new (std::nothrow) DragSideState(this,
		DragSideState::TOP))
	, fDragRState(new (std::nothrow) DragSideState(this,
		DragSideState::RIGHT))
	, fDragBState(new (std::nothrow) DragSideState(this,
		DragSideState::BOTTOM))

	, fDocument(document)
	, fSelection(selection)
	, fObject(NULL)

	, fIgnoreObjectEvents(false)
{
}

// destructor
TransformToolState::~TransformToolState()
{
	SetObject(NULL);
	fSelection->RemoveListener(this);

	delete fPickObjectState;
	delete fDragPivotState;
	delete fDragBoxState;
	delete fDragRotationState;
	delete fDragLTState;
	delete fDragRTState;
	delete fDragRBState;
	delete fDragLBState;
	delete fDragLState;
	delete fDragTState;
	delete fDragRState;
	delete fDragBState;

	delete fPlatformDelegate;
}

// Init()
void
TransformToolState::Init()
{
	if (!fSelection->IsEmpty())
		ObjectSelected(fSelection->SelectableAt(0), NULL);
	fSelection->AddListener(this);
	DragStateViewState::Init();
}

// Cleanup()
void
TransformToolState::Cleanup()
{
	SetObject(NULL);
	DragStateViewState::Cleanup();
	fSelection->RemoveListener(this);
}

// MessageReceived
bool
TransformToolState::MessageReceived(BMessage* message, UndoableEdit** _edit)
{
	bool handled = true;

	switch (message->what) {
		case MSG_ADOPT_OBJECT:
			if (fObject != NULL)
				AdoptObject();
			break;
		default:
			handled = ViewState::MessageReceived(message, _edit);
	}

	return handled;
}

// Draw
void
TransformToolState::Draw(PlatformDrawContext& drawContext)
{
	if (!fOriginalBox.IsValid())
		return;

	double scaleX;
	double scaleY;
	if (!EffectiveTransformation().GetAffineParameters(NULL, NULL, NULL,
		&scaleX, &scaleY, NULL, NULL)) {
		return;
	}

	scaleX *= fView->ZoomLevel();
	scaleY *= fView->ZoomLevel();

	DrawParameters drawParameters(this, fModifiedBox, scaleX, scaleY, Pivot(),
		ViewspaceRotation() != 0.0);
	fPlatformDelegate->Draw(drawContext, drawParameters);
}

// Bounds
BRect
TransformToolState::Bounds() const
{
	BRect bounds(
		min_c(fModifiedBox.left, fModifiedBox.right),
		min_c(fModifiedBox.top, fModifiedBox.bottom),
		max_c(fModifiedBox.left, fModifiedBox.right),
		max_c(fModifiedBox.top, fModifiedBox.bottom));
	TransformObjectToView(&bounds);
	BPoint pivot = Pivot();
	TransformCanvasToView(&pivot);
	if (bounds.left > pivot.x)
		bounds.left = pivot.x;
	if (bounds.right < pivot.x)
		bounds.right = pivot.x;
	if (bounds.top > pivot.y)
		bounds.top = pivot.y;
	if (bounds.bottom < pivot.y)
		bounds.bottom = pivot.y;
	bounds.InsetBy(-10, -10);
	return bounds;
}

// #pragma mark -

// StartTransaction
UndoableEdit*
TransformToolState::StartTransaction(const char* commandName)
{
	return NULL;
}

// DragStateFor
TransformToolState::DragState*
TransformToolState::DragStateFor(BPoint canvasWhere, float zoomLevel) const
{
	if (fObject != NULL) {
		float inset = 7.0 / zoomLevel;

		// First priority has the pivot
		BRect pR(Pivot(), Pivot());

		pR.InsetBy(-inset, -inset);
		if (pR.Contains(canvasWhere))
			return fDragPivotState;

		BPoint where = canvasWhere;
		TransformCanvasToObject(&where);

		// Second priority has the inside of the box, checked with some inset
		// so that the user can drag the whole box when the box is very small
		// and the click is otherwise near enough to a corner.

		BRect iR(fModifiedBox);
		float hInset = min_c(inset, max_c(0, (iR.Width() - inset) / 2.0));
		float vInset = min_c(inset, max_c(0, (iR.Height() - inset) / 2.0));

		iR.InsetBy(hInset, vInset);
		if (iR.Contains(where))
			return fDragBoxState;

		// Next priority have the corners.

		BPoint lt(fModifiedBox.LeftTop());
		BPoint rt(fModifiedBox.RightTop());
		BPoint rb(fModifiedBox.RightBottom());
		BPoint lb(fModifiedBox.LeftBottom());

		TransformObjectToCanvas(&lt);
		TransformObjectToCanvas(&rt);
		TransformObjectToCanvas(&rb);
		TransformObjectToCanvas(&lb);

		float dLT = point_point_distance(canvasWhere, lt);
		float dRT = point_point_distance(canvasWhere, rt);
		float dRB = point_point_distance(canvasWhere, rb);
		float dLB = point_point_distance(canvasWhere, lb);

		float dist = min4(dLT, dRT, dRB, dLB);

		if (dist < inset) {
			if (dist == dLT)
				return fDragLTState;
			if (dist == dRT)
				return fDragRTState;
			if (dist == dRB)
				return fDragRBState;
			if (dist == dLB)
				return fDragLBState;
		}

		// Next priority have the sides.

		float dL = point_stroke_distance(lt, lb, canvasWhere, inset);
		float dR = point_stroke_distance(rt, rb, canvasWhere, inset);
		float dT = point_stroke_distance(lt, rt, canvasWhere, inset);
		float dB = point_stroke_distance(lb, rb, canvasWhere, inset);
		dist = min4(dL, dR, dT, dB);
		if (dist < inset) {
			if (dist == dL)
				return fDragLState;
			else if (dist == dR)
				return fDragRState;
			else if (dist == dT)
				return fDragTState;
			else if (dist == dB)
				return fDragBState;
		}

		// Check inside of the box again.
		if (fModifiedBox.Contains(where))
			return fDragBoxState;

		// Check outside perimeter for rotation.
		BRect rotationRect(fModifiedBox);
		rotationRect.InsetBy(-inset * 3, -inset * 3);
		if (rotationRect.Contains(where))
			return fDragRotationState;
	}

	// If there is still no state, switch to the PickObjectsState
	// and try to find an object. If nothing is picked, unset on mouse down.
	Object* pickedObject = NULL;
	fDocument->RootLayer()->HitTest(canvasWhere, NULL, &pickedObject, true);
	fPickObjectState->SetObject(pickedObject);
	return fPickObjectState;
}

// #pragma mark -

// ObjectSelected
void
TransformToolState::ObjectSelected(const Selectable& selectable,
	const Selection::Controller* controller)
{
	if (controller == this) {
		// ignore changes triggered by ourself
		return;
	}

	Object* object = dynamic_cast<Object*>(selectable.Get());
	SetObject(object);
}

// ObjectDeselected
void
TransformToolState::ObjectDeselected(const Selectable& selectable,
	const Selection::Controller* controller)
{
	if (controller == this) {
		// ignore changes triggered by ourself
		return;
	}

	Object* object = dynamic_cast<Object*>(selectable.Get());
	if (object != NULL && object == fObject)
		SetObject(NULL);
}

// #pragma mark -

// ObjectChanged
void
TransformToolState::ObjectChanged(const Notifier* object)
{
	if (fIgnoreObjectEvents)
		return;

	if (fObject != NULL && object == fObject)
		View()->PostMessage(MSG_ADOPT_OBJECT);
}

// #pragma mark -

// SetObject
void
TransformToolState::SetObject(Object* object, bool modifySelection)
{
	if (fObject == object)
		return;

	if (fObject != NULL) {
		fObject->RemoveListener(this);
		fObject->RemoveReference();
	}

	fObject = object;

	if (fObject != NULL) {
		fObject->AddReference();
		fObject->AddListener(this);

		if (modifySelection)
			fSelection->Select(Selectable(fObject), this);
	} else {
		if (modifySelection)
			fSelection->DeselectAll(this);
	}

	AdoptObject();
}

// AdoptObject
void
TransformToolState::AdoptObject()
{
	BRect box;

	if (fObject != NULL) {
		BoundedObject* boundedObject = dynamic_cast<BoundedObject*>(fObject);
		if (boundedObject != NULL)
			box = boundedObject->Bounds();
		else {
			// TODO: If the transformed object is not a BoundedObject, we
			// could also display ourselfs differently, for example with
			// arrows like in Maya.
			box = BRect(0, 0, 100, 100);
		}

		fOriginalTransformation = *fObject;
		fParentGlobalTransformation.Reset();

		SetObjectToCanvasTransformation(fObject->Transformation());
		Object* parent = fObject->Parent();
		if (parent != NULL)
			fParentGlobalTransformation = parent->Transformation();
	} else {
		SetObjectToCanvasTransformation(Transformable());
		box = BRect();
	}

	SetBox(box);
}


// SetBox
void
TransformToolState::SetBox(const BRect& box)
{
	SetAdditionalTransformation(Transformable());

	fOriginalBox = box;
	fModifiedBox = box;

	fPivot = BPoint((box.left + box.right) / 2, (box.top + box.bottom) / 2);
	TransformObjectToCanvas(&fPivot);

	fRotation = 0.0;

	UpdateAdditionalTransformation();
}

// SetModifiedBox
void
TransformToolState::SetModifiedBox(const BRect& box, bool update)
{
	if (fModifiedBox == box)
		return;
	fModifiedBox = box;
	if (update)
		UpdateAdditionalTransformation();
}

// SetPivot
void
TransformToolState::SetPivot(const BPoint& pivot, bool update)
{
	if (fPivot == pivot)
		return;

	// If we already have a rotation, just setting the pivot would add an
	// undesired translation. We have to compute the translation to compensate.
	if (LocalRotation() != 0) {
		// Originally we had: G' = RGB.
		// We change the rotation to R' and want to compensate with an
		// additional translation T: G' = R'GTB.
		// => RGB = R'GTB
		// => T = (G^-1)(R'^-1)RG
		Transformable globalTransformation(fOriginalTransformation);
		globalTransformation.Multiply(fParentGlobalTransformation);

		Transformable pivotTranslation(globalTransformation);
		pivotTranslation.RotateBy(fPivot, LocalRotation());
		pivotTranslation.RotateBy(pivot, -LocalRotation());
		pivotTranslation.MultiplyInverse(globalTransformation);

		fModifiedBox.OffsetBy(pivotTranslation.Translation());
	}

	fPivot = pivot;
	if (update)
		UpdateAdditionalTransformation();
}

// TranslationX
float
TransformToolState::TranslationX() const
{
	return fModifiedBox.left - fOriginalBox.left;
}

// TranslationY
float
TransformToolState::TranslationY() const
{
	return fModifiedBox.top - fOriginalBox.top;
}

// Translation
BPoint
TransformToolState::Translation() const
{
	return BPoint(TranslationX(), TranslationY());
}

// SetLocalRotation
void
TransformToolState::SetLocalRotation(double rotation)
{
	if (fRotation == rotation)
		return;
	fRotation = rotation;
	UpdateAdditionalTransformation();
}

// LocalXScale
double
TransformToolState::LocalXScale() const
{
	if (fOriginalBox.Width() == 0.0)
		return 1.0;
	return fModifiedBox.Width() / fOriginalBox.Width();
}

// LocalYScale
double
TransformToolState::LocalYScale() const
{
	if (fOriginalBox.Height() == 0.0)
		return 1.0;
	return fModifiedBox.Height() / fOriginalBox.Height();
}

// UpdateAdditionalTransformation
Transformable
TransformToolState::UpdateAdditionalTransformation()
{
	// Only local rotation is treated as additional transformation.
	Transformable additionalTransform;
	additionalTransform.RotateBy(Pivot(), LocalRotation());
	SetAdditionalTransformation(additionalTransform);
	UpdateBounds();

	// Translation and scale are only applied to the object.
// NOTE [bonefish]: Here's the math:
// L: original local object transformation
// P: original parent to canvas transformation
// G = PL: original object to canvas transformation
// B: bounding box transformation in the object's coordinate system, i.e. the
//    transformation that transforms fOriginalBox to fModifiedBox (consisting
//    of a scale and a translation)
// R: the rotation on the canvas (i.e. around the pivot transformed to the
//    canvas) -- correctly computed above as additionalTransform
// G' = RPLB: the modified object to canvas transformation
// L': the modified local object transformation, such that G' = PL'
// Hence we get PL' = RPLB => L' = (P^-1)RPLB
// The correct B is:
	Transformable transform;
	transform.ScaleBy(BPoint(fOriginalBox.left, fOriginalBox.top),
		LocalXScale(), LocalYScale());
	transform.TranslateBy(Translation());

	if (fObject != NULL && fDocument->WriteLock()) {
		// TODO: Use UndoableEdit!
		// TODO: There will be two different code paths to transform objects.
		// In the first case, we may transform multiple objects and use their
		// global bounding box, and generally operate in global coordinate
		// space. Then the code below needs to be used to set the transformation
		// on each affected object.
		// In the second case, we may adopt the local transformation of an
		// object (or multiple objects with the same rotation perhaps) and then
		// we operate in some kind of local transformation mode. Then the code
		// which is now effective can be used.
// NOTE [bonefish]: The first case in the TODO is correct in principle and so is
// the following code. However, it assumes that the complete bounding box
// transformation is applied at the very end (i.e. as "additional
// transformation") or IOW, G' = B'PL with B' being a complete bounding box
// transformation (i.e. consisting of rotation, scale, and translation) applied
// in the canvas coordinate system. Hence it doesn't match the way it currently
// works.
//		Transformable newTransformation(fOriginalTransformation);
//		newTransformation.Multiply(fParentGlobalTransformation);
//		newTransformation.Multiply(transform);
//		newTransformation.MultiplyInverse(fParentGlobalTransformation);
//		fObject->SetTransformable(newTransformation);

// NOTE [bonefish]: This code computes L' = LB'' with B'' being a complete
// bounding box transformation (i.e. consisting of rotation, scale, and
// translation) applied in the object's coordinate system (i.e. G' = PLB'').
// Hence it doesn't match the way it currently works either.
//		Transformable newTransformation(transform);
//		newTransformation.Multiply(fOriginalTransformation);
//		fObject->SetTransformable(newTransformation);

// NOTE [bonefish]: This matches the way it currently works (L' = (P^-1)RPLB).
// It requires "transform" to be computed as written above.
// The effect is that the rendering of bounding box and object match, but
// apparently the drag states don't set the parameters correctly, since
// moving the box after rotating it makes it jump weirdly. I haven't tried to
// analyze it, but I suppose the previous transformation isn't taken into
// account correctly when computing the new modified box and pivot.
// If I understand the above TODO correctly, it shall eventually be possible to
// transform multiple objects. I don't really understand why the TODO suggests
// alternative cases. IMO the only thing that makes sense with multiple objects
// is the G' = B'PL approach, since P and/or L may be different for each object,
// so that with any approach using data (transformation or box) in the object's
// coordinate system that data would have to be tracked for each object. IMO the
// controls should display/modify B' anyway -- e.g. I find it rather confusing
// how moving a rotated object affects the X/Y translation, since the object to
// canvas transformation for that object might not be obvious.
		Transformable newTransformation(transform);
		newTransformation.Multiply(fOriginalTransformation);
		newTransformation.Multiply(fParentGlobalTransformation);
		newTransformation.Multiply(additionalTransform);
		newTransformation.MultiplyInverse(fParentGlobalTransformation);

		fIgnoreObjectEvents = true;
		View()->PerformEdit(new(std::nothrow) TransformObjectEdit(
			fObject, newTransformation));
		fIgnoreObjectEvents = false;

		fDocument->WriteUnlock();
	}

	if (fConfigViewMessenger.IsValid()) {
		BMessage message(MSG_TRANSFORMATION_CHANGED);
		message.AddFloat("pivot x", Pivot().x);
		message.AddFloat("pivot y", Pivot().y);
		message.AddFloat("translation x", TranslationX());
		message.AddFloat("translation y", TranslationY());
		message.AddDouble("rotation", LocalRotation());
		message.AddDouble("scale x", LocalXScale());
		message.AddDouble("scale y", LocalYScale());
		fConfigViewMessenger.SendMessage(&message);
	}

	return transform;
}

