#ifndef _RECTANGLETOOLSTATE_CPP_
#define _RECTANGLETOOLSTATE_CPP_

/*
 * Copyright 2012-2013 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "RectangleToolState.h"
#include "RectangleToolStatePlatformDelegate.h"

#include <Cursor.h>
#include <MessageRunner.h>
#include <Rect.h>

#include <new>

#include <agg_math.h>

#include "EditManager.h"
#include "CompoundEdit.h"
#include "CurrentColor.h"
#include "cursors.h"
#include "Document.h"
#include "Layer.h"
#include "ObjectAddedEdit.h"
#include "Rect.h"
#include "RectangleAreaEdit.h"
#include "RectangleTool.h"
#include "StyleSetFillPaintEdit.h"
#include "support.h"
#include "TransformObjectEdit.h"
#include "ui_defines.h"

enum {
	MSG_UPDATE_BOUNDS	= 'ptub',
};

// CreateRectangleState
class RectangleToolState::CreateRectangleState
	: public DragStateViewState::DragState {
public:
	CreateRectangleState(RectangleToolState* parent)
		: DragState(parent)
		, fParent(parent)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		BPoint originalOrigin(origin);

		// Create rectangle
		if (!fParent->CreateRectangle(origin))
			return;

		fParent->TransformCanvasToObject(&origin);
		fOrigin = origin;
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		if (fParent->fRectangle == NULL)
			return;
		
		fParent->TransformCanvasToObject(&current);
		BRect rect(
			std::min(fOrigin.x, current.x),
			std::min(fOrigin.y, current.y),
			std::max(fOrigin.x, current.x),
			std::max(fOrigin.y, current.y));
		
		fParent->fRectangle->SetArea(rect);
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		return BCursor(kRectCursor);
	}

	virtual const char* CommandName() const
	{
		return "Create rectangle";
	}

private:
	RectangleToolState*		fParent;
};

class RectangleToolState::DragBoxState : public DragStateViewState::DragState {
public:
	DragBoxState(RectangleToolState* parent)
		: DragState(parent)
		, fParent(parent)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		if (fParent->fRectangle == NULL)
			return;

		fParent->TransformCanvasToObject(&origin);
		DragState::SetOrigin(origin);

		fStartArea = fParent->fRectangle->Area();
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		if (fParent->fRectangle == NULL)
			return;

		fParent->TransformCanvasToObject(&current);
		BPoint offset = current - fOrigin;

		BRect area = fStartArea;
		area.OffsetBy(offset);

		fParent->SetArea(area);
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		return BCursor(B_CURSOR_ID_MOVE);
	}

	virtual const char* CommandName() const
	{
		return "Move rectangle";
	}

private:
	RectangleToolState*	fParent;
	BRect				fStartArea;
};

// DragCornerState
class RectangleToolState::DragCornerState
	: public DragStateViewState::DragState {
public:
	enum Corner {
		LEFT_TOP,
		RIGHT_TOP,
		LEFT_BOTTOM,
		RIGHT_BOTTOM,
	};

	DragCornerState(RectangleToolState* parent)
		: DragState(parent)
		, fParent(parent)
		, fCorner(LEFT_TOP)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		if (fParent->fRectangle == NULL)
			return;

		fParent->TransformCanvasToObject(&origin);
		fOrigin = origin;
		
		fStartArea = fParent->fRectangle->Area();
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		if (fParent->fRectangle == NULL)
			return;

		fParent->TransformCanvasToObject(&current);
		
		BPoint offset = current - fOrigin;
		
		// TODO: Support proportional resizing!
		
		BRect area = fStartArea;
		switch (fCorner) {
			case LEFT_TOP:
				area.left = fStartArea.left + offset.x;
				area.top = fStartArea.top + offset.y;
				break;
			case RIGHT_TOP:
				area.right = fStartArea.right + offset.x;
				area.top = fStartArea.top + offset.y;
				break;
			case LEFT_BOTTOM:
				area.left = fStartArea.left + offset.x;
				area.bottom = fStartArea.bottom + offset.y;
				break;
			case RIGHT_BOTTOM:
				area.right = fStartArea.right + offset.x;
				area.bottom = fStartArea.bottom + offset.y;
				break;
		}

		if (area.left > area.right) {
			float temp = area.left;
			area.left = area.right;
			area.right = temp;
		}
		
		if (area.top > area.bottom) {
			float temp = area.top;
			area.top = area.bottom;
			area.bottom = temp;
		}

		fParent->SetArea(area);
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		float rotation = fmod(360.0 - fParent->ViewspaceRotation() + 22.5,
			180.0);

		BCursorID cursorID = B_CURSOR_ID_MOVE;
		if (rotation < 45.0) {
			switch (fCorner) {
				case LEFT_TOP:
				case RIGHT_BOTTOM:
					cursorID = B_CURSOR_ID_RESIZE_NORTH_WEST_SOUTH_EAST;
					break;
				case RIGHT_TOP:
				case LEFT_BOTTOM:
					cursorID = B_CURSOR_ID_RESIZE_NORTH_EAST_SOUTH_WEST;
					break;
			}
		} else if (rotation < 90.0) {
			switch (fCorner) {
				case LEFT_TOP:
				case RIGHT_BOTTOM:
					cursorID = B_CURSOR_ID_RESIZE_EAST_WEST;
					break;
				case RIGHT_TOP:
				case LEFT_BOTTOM:
					cursorID = B_CURSOR_ID_RESIZE_NORTH_SOUTH;
					break;
			}
		} else if (rotation < 135.0) {
			switch (fCorner) {
				case LEFT_TOP:
				case RIGHT_BOTTOM:
					cursorID = B_CURSOR_ID_RESIZE_NORTH_EAST_SOUTH_WEST;
					break;
				case RIGHT_TOP:
				case LEFT_BOTTOM:
					cursorID = B_CURSOR_ID_RESIZE_NORTH_WEST_SOUTH_EAST;
					break;
			}
		} else {
			switch (fCorner) {
				case LEFT_TOP:
				case RIGHT_BOTTOM:
					cursorID = B_CURSOR_ID_RESIZE_NORTH_SOUTH;
					break;
				case RIGHT_TOP:
				case LEFT_BOTTOM:
					cursorID = B_CURSOR_ID_RESIZE_EAST_WEST;
					break;
			}
		}
		return BCursor(cursorID);
	}

	virtual const char* CommandName() const
	{
		return "Drag rectangle corner";
	}

	void SetCorner(Corner corner)
	{
		fCorner = corner;
	}

private:
	RectangleToolState*		fParent;
	Corner					fCorner;
	BRect					fStartArea;
};

// DragSideState
class RectangleToolState::DragSideState
	: public DragStateViewState::DragState {
public:
	enum Side {
		LEFT,
		RIGHT,
		TOP,
		BOTTOM,
	};

	DragSideState(RectangleToolState* parent)
		: DragState(parent)
		, fParent(parent)
		, fSide(LEFT)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		if (fParent->fRectangle == NULL)
			return;

		fParent->TransformCanvasToObject(&origin);
		fOrigin = origin;
		
		fStartArea = fParent->fRectangle->Area();
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		if (fParent->fRectangle == NULL)
			return;

		fParent->TransformCanvasToObject(&current);
		
		BPoint offset = current - fOrigin;
		
		// TODO: Support proportional resizing!
		
		BRect area = fStartArea;
		switch (fSide) {
			case LEFT:
				area.left = fStartArea.left + offset.x;
				break;
			case RIGHT:
				area.right = fStartArea.right + offset.x;
				break;
			case TOP:
				area.top = fStartArea.top + offset.y;
				break;
			case BOTTOM:
				area.bottom = fStartArea.bottom + offset.y;
				break;
		}

		if (area.left > area.right) {
			float temp = area.left;
			area.left = area.right;
			area.right = temp;
		}
		
		if (area.top > area.bottom) {
			float temp = area.top;
			area.top = area.bottom;
			area.bottom = temp;
		}

		fParent->SetArea(area);
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
		return "Drag rectangle side";
	}

	void SetSide(Side side)
	{
		fSide = side;
	}

private:
	RectangleToolState*		fParent;
	Side					fSide;
	BRect					fStartArea;
};

// #pragma mark -

// constructor
RectangleToolState::RectangleToolState(StateView* view, RectangleTool* tool,
		Document* document, Selection* selection, CurrentColor* color,
		const BMessenger& configView)
	: DragStateViewState(view)

	, fTool(tool)

	, fPlatformDelegate(new PlatformDelegate(this))

	, fCreateRectangleState(new(std::nothrow) CreateRectangleState(this))
	, fDragBoxState(new(std::nothrow) DragBoxState(this))
	, fDragCornerState(new(std::nothrow) DragCornerState(this))
	, fDragSideState(new(std::nothrow) DragSideState(this))

	, fDocument(document)
	, fSelection(selection)
	, fCurrentColor(color)

	, fConfigViewMessenger(configView)

	, fInsertionLayer(NULL)
	, fInsertionIndex(-1)

	, fRectangle(NULL)
	
	, fRoundCornerRadius(0.0)
	
	, fIgnoreColorNotifiactions(false)
{
	// TODO: Find a way to change this later...
	SetInsertionInfo(fDocument->RootLayer(),
		fDocument->RootLayer()->CountObjects());

	fCurrentColor->AddListener(this);
}

// destructor
RectangleToolState::~RectangleToolState()
{
	SetRectangle(NULL);
	fCurrentColor->RemoveListener(this);
	fSelection->RemoveListener(this);

	delete fCreateRectangleState;
	delete fDragBoxState;
	delete fDragCornerState;
	delete fDragSideState;

	SetInsertionInfo(NULL, -1);

	delete fPlatformDelegate;
}

// Init
void
RectangleToolState::Init()
{
	if (!fSelection->IsEmpty())
		ObjectSelected(fSelection->SelectableAt(0), NULL);
	fSelection->AddListener(this);
	DragStateViewState::Init();
}

// Cleanup
void
RectangleToolState::Cleanup()
{
	SetRectangle(NULL);
	DragStateViewState::Cleanup();
	fSelection->RemoveListener(this);
}

// MessageReceived
bool
RectangleToolState::MessageReceived(BMessage* message, UndoableEdit** _edit)
{
	bool handled = true;

	switch (message->what) {
		case MSG_UPDATE_BOUNDS:
			if (fRectangle != NULL) {
				_AdoptRectanglePaint();
				SetObjectToCanvasTransformation(fRectangle->Transformation());
				UpdateBounds();
				UpdateDragState();
			}

			handled = true;
			break;
		default:
			handled = DragStateViewState::MessageReceived(message, _edit);
	}

	return handled;
}

// #pragma mark -

// MouseUp
UndoableEdit*
RectangleToolState::MouseUp()
{
	return DragStateViewState::MouseUp();
}

// #pragma mark -

// ModifiersChanged
void
RectangleToolState::ModifiersChanged(uint32 modifiers)
{
	DragStateViewState::ModifiersChanged(modifiers);
	UpdateDragState();
}

// HandleKeyDown
bool
RectangleToolState::HandleKeyDown(const StateView::KeyEvent& event,
	UndoableEdit** _edit)
{
	bool handled = false;

	if (fRectangle != NULL) {
		switch (event.key) {
			case B_UP_ARROW:
				break;
			case B_DOWN_ARROW:
				break;
			case B_LEFT_ARROW:
				break;
			case B_RIGHT_ARROW:
				break;

			default:
				break;
		}
	}

	if (!handled)
		handled = DragStateViewState::HandleKeyDown(event, _edit);

	return handled;
}

// HandleKeyUp
bool
RectangleToolState::HandleKeyUp(const StateView::KeyEvent& event,
	UndoableEdit** _edit)
{
	return DragStateViewState::HandleKeyUp(event, _edit);
}

// #pragma mark -

// Draw
void
RectangleToolState::Draw(PlatformDrawContext& drawContext)
{
	_DrawControls(drawContext);
}

// Bounds
BRect
RectangleToolState::Bounds() const
{
	BRect bounds(LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN);

	if (fRectangle != NULL) {
		bounds = fRectangle->Bounds();
		TransformObjectToView(&bounds);
		bounds.InsetBy(-15, -15);
	}

	return bounds;
}

// #pragma mark -

// StartTransaction
UndoableEdit*
RectangleToolState::StartTransaction(const char* editName)
{
	return NULL;
}

// DragStateFor
RectangleToolState::DragState*
RectangleToolState::DragStateFor(BPoint canvasWhere, float zoomLevel) const
{
	if (fRectangle != NULL) {
		float inset = 10.0 / zoomLevel;

		BPoint objectWhere(canvasWhere);
		TransformCanvasToObject(&objectWhere);
		
		BRect area = fRectangle->Area();
		BPoint lt(area.LeftTop());
		BPoint rt(area.RightTop());
		BPoint lb(area.LeftBottom());
		BPoint rb(area.RightBottom());
		
		// Second priority has the inside of the box, checked with some inset
		// so that the user can drag the whole box when the box is very small
		// and the click is otherwise near enough to a corner.

		BRect iR(area);
		float hInset = min_c(inset, max_c(0, (iR.Width() - inset) / 2.0));
		float vInset = min_c(inset, max_c(0, (iR.Height() - inset) / 2.0));

		iR.InsetBy(hInset, vInset);
		if (iR.Contains(objectWhere))
			return fDragBoxState;

		float distLT = point_point_distance(lt, objectWhere);
		float distRT = point_point_distance(rt, objectWhere);
		float distLB = point_point_distance(lb, objectWhere);
		float distRB = point_point_distance(rb, objectWhere);

		float dist = min4(distLT, distRT, distLB, distRB);
		if (dist < inset) {
			if (dist == distLT)
				fDragCornerState->SetCorner(DragCornerState::LEFT_TOP);
			else if (dist == distRT)
				fDragCornerState->SetCorner(DragCornerState::RIGHT_TOP);
			else if (dist == distLB)
				fDragCornerState->SetCorner(DragCornerState::LEFT_BOTTOM);
			else if (dist == distRB)
				fDragCornerState->SetCorner(DragCornerState::RIGHT_BOTTOM);
			return fDragCornerState;
		}

		// Next priority have the sides.

		float dL = point_stroke_distance(lt, lb, objectWhere, inset);
		float dR = point_stroke_distance(rt, rb, objectWhere, inset);
		float dT = point_stroke_distance(lt, rt, objectWhere, inset);
		float dB = point_stroke_distance(lb, rb, objectWhere, inset);
		dist = min4(dL, dR, dT, dB);
		if (dist < inset) {
			if (dist == dL)
				fDragSideState->SetSide(DragSideState::LEFT);
			else if (dist == dR)
				fDragSideState->SetSide(DragSideState::RIGHT);
			else if (dist == dT)
				fDragSideState->SetSide(DragSideState::TOP);
			else if (dist == dB)
				fDragSideState->SetSide(DragSideState::BOTTOM);
			return fDragSideState;
		}

		// Check inside of the box again.
		if (area.Contains(objectWhere))
			return fDragBoxState;

		return NULL;
	}
	
	return fCreateRectangleState;
}

// #pragma mark -

// ObjectSelected
void
RectangleToolState::ObjectSelected(const Selectable& selectable,
	const Selection::Controller* controller)
{
	if (controller == this) {
		// ignore changes triggered by ourself
		return;
	}

	Rect* rectangle = dynamic_cast<Rect*>(selectable.Get());
	SetRectangle(rectangle);
}

// ObjectDeselected
void
RectangleToolState::ObjectDeselected(const Selectable& selectable,
	const Selection::Controller* controller)
{
	if (controller == this) {
		// ignore changes triggered by ourself
		return;
	}

	Rect* rectangle = dynamic_cast<Rect*>(selectable.Get());
	if (rectangle == fRectangle)
		SetRectangle(NULL);
}

// #pragma mark -

// ObjectChanged
void
RectangleToolState::ObjectChanged(const Notifier* object)
{
	if (fRectangle != NULL && object == fRectangle) {
		// Update asynchronously, since the notification may arrive on
		// the wrong thread.
		View()->PostMessage(MSG_UPDATE_BOUNDS);
	}

	if (object == fCurrentColor && !fIgnoreColorNotifiactions) {
		AutoWriteLocker _(View()->Locker());
		
		if (fRectangle != NULL) {
			Style* style = fRectangle->Style();
			View()->PerformEdit(new(std::nothrow) StyleSetFillPaintEdit(style,
				Paint(fCurrentColor->Color())));
		}
	}
}

// #pragma mark -

// SetInsertionInfo
void
RectangleToolState::SetInsertionInfo(Layer* layer, int32 index)
{
	if (layer != fInsertionLayer) {
		if (layer != NULL)
			layer->AddReference();
		if (fInsertionLayer != NULL)
			fInsertionLayer->RemoveReference();
		fInsertionLayer = layer;
	}
	fInsertionIndex = index;
}

// CreateRectangle
bool
RectangleToolState::CreateRectangle(BPoint canvasLocation)
{
	if (fInsertionLayer == NULL) {
		fprintf(stderr, "RectangleToolState::MouseDown(): No insertion layer "
			"specified\n");
		return false;
	}

	Rect* rectangle = new(std::nothrow) Rect(BRect(0, 0, 0, 0),
		fCurrentColor->Color());
	Style* style = new(std::nothrow) Style();
	if (rectangle == NULL || style == NULL) {
		fprintf(stderr, "RectangleToolState::CreateRectangle(): Failed to allocate "
			"Rect or Style. Out of memory\n");
		delete rectangle;
		delete style;
		return false;
	}

	style->SetFillPaint(Paint(fCurrentColor->Color()));
	rectangle->SetStyle(style);
	style->RemoveReference();

	rectangle->TranslateBy(canvasLocation);

	rectangle->SetRoundCornerRadius(fRoundCornerRadius);

	if (fInsertionIndex < 0)
		fInsertionIndex = 0;
	if (fInsertionIndex > fInsertionLayer->CountObjects())
		fInsertionIndex = fInsertionLayer->CountObjects();

	if (!fInsertionLayer->AddObject(rectangle, fInsertionIndex)) {
		fprintf(stderr, "RectangleToolState::CreateRectangle(): Failed to add "
			"Rect to Layer. Out of memory\n");
		rectangle->RemoveReference();
		return false;
	}

	fInsertionIndex++;

	SetRectangle(rectangle, true);

	// Our reference to this object was transferred to the Layer

	View()->PerformEdit(new(std::nothrow) ObjectAddedEdit(rectangle,
		fSelection));


	return true;
}

// SetRectangle
void
RectangleToolState::SetRectangle(Rect* rectangle, bool modifySelection)
{
	if (fRectangle == rectangle)
		return;

	if (fRectangle != NULL) {
		fRectangle->RemoveListener(this);
		fRectangle->RemoveReference();
		fTool->NotifyConfirmableEditFinished();
	}

	fRectangle = rectangle;

	if (fRectangle != NULL) {
		fRectangle->AddReference();
		fRectangle->AddListener(this);
		fTool->NotifyConfirmableEditStarted();
	}

	if (rectangle != NULL) {
		if (modifySelection)
			fSelection->Select(Selectable(rectangle), this);

		SetObjectToCanvasTransformation(fRectangle->Transformation());
		ObjectChanged(rectangle);

		_AdoptRectanglePaint();
	} else {
		if (modifySelection)
			fSelection->DeselectAll(this);
//		SetObjectToCanvasTransformation(Transformable());
		UpdateBounds();
	}
}

// SetArea
void
RectangleToolState::SetArea(const BRect& area)
{
	View()->PerformEdit(
		new(std::nothrow) RectangleAreaEdit(fRectangle, area, fSelection)
	);
}

// SetRoundCornerRadius
void
RectangleToolState::SetRoundCornerRadius(double radius)
{
	if (fRoundCornerRadius == radius)
		return;
	
	fRoundCornerRadius = radius;
	
	// TODO: UndoableEdit
	if (fRectangle != NULL)
		fRectangle->SetRoundCornerRadius(fRoundCornerRadius);
}

// Confirm
void
RectangleToolState::Confirm()
{
	SetRectangle(NULL, true);
}

// Cancel
void
RectangleToolState::Cancel()
{
	// TODO: Restore Rectangle to when editing began
	SetRectangle(NULL, true);
}

// #pragma mark - private

// _DrawControls
void
RectangleToolState::_DrawControls(PlatformDrawContext& drawContext)
{
	if (fRectangle == NULL)
		return;

	BRect box = fRectangle->Area();
	
	BPoint lt(box.LeftTop());
	BPoint rt(box.RightTop());
	BPoint rb(box.RightBottom());
	BPoint lb(box.LeftBottom());

	TransformObjectToView(&lt, true);
	TransformObjectToView(&rt, true);
	TransformObjectToView(&rb, true);
	TransformObjectToView(&lb, true);

	fPlatformDelegate->DrawBox(drawContext, lt, rt, rb, lb, ZoomLevel());
}

// _AdoptRectanglePaint
void
RectangleToolState::_AdoptRectanglePaint()
{
	Style* style = fRectangle->Style();
	Paint* fillPaint = style->FillPaint();
	if (fillPaint->Type() == Paint::COLOR) {
		fIgnoreColorNotifiactions = true;
		fCurrentColor->SetColor(fillPaint->Color());
		fIgnoreColorNotifiactions = false;
	}
}

#endif	// _RECTANGLETOOLSTATE_CPP_
