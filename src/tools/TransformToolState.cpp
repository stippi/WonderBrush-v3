/*
 * Copyright 2009 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "TransformToolState.h"

#include <Cursor.h>
#include <Shape.h>

#include <new>

#include <agg_math.h>

#include "Command.h"
#include "cursors.h"
#include "support.h"


class TransformToolState::DragBoxState : public DragStateViewState::DragState {
public:
	DragBoxState(TransformToolState* parent)
		:
		DragState(parent),
		fParent(parent)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		fParent->TransformCanvasToObject(&origin);
		DragState::SetOrigin(origin);
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		fParent->TransformCanvasToObject(&current);
		BPoint offset = current - fOrigin;
		fOrigin = current;
		BRect box = fParent->ModifiedBox();
		box.OffsetBy(offset);
		fParent->SetModifiedBox(box);
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
#ifdef __HAIKU__
		return BCursor(B_CURSOR_ID_MOVE);
#else
		return BCursor(kMoveCursor);
#endif
	}

	virtual const char* CommandName() const
	{
		return "Move";
	}

private:
	TransformToolState*	fParent;
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
		:
		DragState(parent),
		fParent(parent),
		fCorner(corner)
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
#ifdef __HAIKU__
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
#else // (!__HAIKU__)
		const uint8* cursorData = kMoveCursor;
		if (rotation < 45.0) {
			switch (fCorner) {
				case LEFT_TOP:
				case RIGHT_BOTTOM:
					if (flipX) {
						cursorData = flipY
							? kLeftTopRightBottomCursor
							: kLeftBottomRightTopCursor;
					} else {
						cursorData = flipY
							? kLeftBottomRightTopCursor
							: kLeftTopRightBottomCursor;
					}
					break;
				case RIGHT_TOP:
				case LEFT_BOTTOM:
					if (flipX) {
						cursorData = flipY
							? kLeftBottomRightTopCursor
							: kLeftTopRightBottomCursor;
					} else {
						cursorData = flipY
							? kLeftTopRightBottomCursor
							: kLeftBottomRightTopCursor;
					}
					break;
			}
		} else if (rotation < 90.0) {
			switch (fCorner) {
				case LEFT_TOP:
				case RIGHT_BOTTOM:
					if (flipX)
						cursorData = flipY ? kLeftRightCursor : kUpDownCursor;
					else
						cursorData = flipY ? kUpDownCursor : kLeftRightCursor;
					break;
				case RIGHT_TOP:
				case LEFT_BOTTOM:
					if (flipX)
						cursorData = flipY ? kUpDownCursor : kLeftRightCursor;
					else
						cursorData = flipY ? kLeftRightCursor : kUpDownCursor;
					break;
			}
		} else if (rotation < 135.0) {
			switch (fCorner) {
				case LEFT_TOP:
				case RIGHT_BOTTOM:
					if (flipX) {
						cursorData = flipY
							? kLeftBottomRightTopCursor
							: kLeftTopRightBottomCursor;
					} else {
						cursorData = flipY
							? kLeftTopRightBottomCursor
							: kLeftBottomRightTopCursor;
					}
					break;
				case RIGHT_TOP:
				case LEFT_BOTTOM:
					if (flipX) {
						cursorData = flipY
							? kLeftTopRightBottomCursor
							: kLeftBottomRightTopCursor;
					} else {
						cursorData = flipY
							? kLeftBottomRightTopCursor
							: kLeftTopRightBottomCursor;
					}
					break;
			}
		} else {
			switch (fCorner) {
				case LEFT_TOP:
				case RIGHT_BOTTOM:
					if (flipX)
						cursorData = flipY ? kUpDownCursor : kLeftRightCursor;
					else
						cursorData = flipY ? kLeftRightCursor : kUpDownCursor;
					break;
				case RIGHT_TOP:
				case LEFT_BOTTOM:
					if (flipX)
						cursorData = flipY ? kLeftRightCursor : kUpDownCursor;
					else
						cursorData = flipY ? kUpDownCursor : kLeftRightCursor;
					break;
			}
		}
		return BCursor(cursorData);
#endif // (!__HAIKU__)
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
		:
		DragState(parent),
		fParent(parent),
		fSide(side)
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
#ifdef __HAIKU__
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
#else // (!__HAIKU__)
		const uint8* cursorData = kMoveCursor;
		if (rotation < 45.0) {
			switch (fSide) {
				case LEFT:
				case RIGHT:
					cursorData = kLeftRightCursor;
					break;
				case TOP:
				case BOTTOM:
					cursorData = kUpDownCursor;
					break;
			}
		} else if (rotation < 90.0) {
			switch (fSide) {
				case LEFT:
				case RIGHT:
					cursorData = kLeftBottomRightTopCursor;
					break;
				case TOP:
				case BOTTOM:
					cursorData = kLeftTopRightBottomCursor;
					break;
			}
		} else if (rotation < 135.0) {
			switch (fSide) {
				case LEFT:
				case RIGHT:
					cursorData = kUpDownCursor;
					break;
				case TOP:
				case BOTTOM:
					cursorData = kLeftRightCursor;
					break;
			}
		} else {
			switch (fSide) {
				case LEFT:
				case RIGHT:
					cursorData = kLeftTopRightBottomCursor;
					break;
				case TOP:
				case BOTTOM:
					cursorData = kLeftBottomRightTopCursor;
					break;
			}
		}
		return BCursor(cursorData);
#endif // (!__HAIKU__)
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


// #pragma mark -


// constructor
TransformToolState::TransformToolState(StateView* view, const BRect& box)
	:
	DragStateViewState(view),
	fOriginalBox(box),
	fModifiedBox(box),

	fDragBoxState(new (std::nothrow) DragBoxState(this)),

	fDragLTState(new (std::nothrow) DragCornerState(this,
		DragCornerState::LEFT_TOP)),
	fDragRTState(new (std::nothrow) DragCornerState(this,
		DragCornerState::RIGHT_TOP)),
	fDragRBState(new (std::nothrow) DragCornerState(this,
		DragCornerState::RIGHT_BOTTOM)),
	fDragLBState(new (std::nothrow) DragCornerState(this,
		DragCornerState::LEFT_BOTTOM)),

	fDragLState(new (std::nothrow) DragSideState(this, DragSideState::LEFT)),
	fDragTState(new (std::nothrow) DragSideState(this, DragSideState::TOP)),
	fDragRState(new (std::nothrow) DragSideState(this, DragSideState::RIGHT)),
	fDragBState(new (std::nothrow) DragSideState(this, DragSideState::BOTTOM))
{
}

// destructor
TransformToolState::~TransformToolState()
{
	delete fDragBoxState;
	delete fDragLTState;
	delete fDragRTState;
	delete fDragRBState;
	delete fDragLBState;
	delete fDragLState;
	delete fDragTState;
	delete fDragRState;
	delete fDragBState;
}

// Draw
void
TransformToolState::Draw(BView* view, BRect updateRect)
{
	double scaleX;
	double scaleY;
	if (!EffectiveTransformation().GetAffineParameters(NULL, NULL, NULL,
		&scaleX, &scaleY, NULL, NULL)) {
		return;
	}

	scaleX *= fView->ZoomLevel();
	scaleY *= fView->ZoomLevel();

	float insetX = 6 / scaleX;
	float insetY = 6 / scaleY;
	float insetFill1X = 5.2 / scaleX;
	float insetFill1Y = 5.2 / scaleY;
	float insetFill2X = 0.8 / scaleX;
	float insetFill2Y = 0.8 / scaleY;

	BPoint lt(
		min_c(fModifiedBox.left, fModifiedBox.right),
		min_c(fModifiedBox.top, fModifiedBox.bottom));
	BPoint rt(
		max_c(fModifiedBox.left, fModifiedBox.right),
		min_c(fModifiedBox.top, fModifiedBox.bottom));
	BPoint rb(
		max_c(fModifiedBox.left, fModifiedBox.right),
		max_c(fModifiedBox.top, fModifiedBox.bottom));
	BPoint lb(
		min_c(fModifiedBox.left, fModifiedBox.right),
		max_c(fModifiedBox.top, fModifiedBox.bottom));

	BPoint lt1(lt.x - insetX, lt.y);
	BPoint lt2(lt.x - insetX, lt.y - insetY);
	BPoint lt3(lt.x, lt.y - insetY);

	BPoint rt1(rt.x, rt.y - insetY);
	BPoint rt2(rt.x + insetX, rt.y - insetY);
	BPoint rt3(rt.x + insetX, rt.y);

	BPoint rb1(rb.x + insetX, rb.y);
	BPoint rb2(rb.x + insetX, rb.y + insetY);
	BPoint rb3(rb.x, rb.y + insetY);

	BPoint lb1(lb.x, lb.y + insetY);
	BPoint lb2(lb.x - insetX, lb.y + insetY);
	BPoint lb3(lb.x - insetX, lb.y);

	BPoint ltF0(lt.x - insetFill2X, lt.y - insetFill2Y);
	BPoint ltF1(lt.x - insetFill1X, lt.y - insetFill2Y);
	BPoint ltF2(lt.x - insetFill1X, lt.y - insetFill1Y);
	BPoint ltF3(lt.x - insetFill2X, lt.y - insetFill1Y);

	BPoint rtF0(rt.x + insetFill2X, rt.y - insetFill2Y);
	BPoint rtF1(rt.x + insetFill2X, rt.y - insetFill1Y);
	BPoint rtF2(rt.x + insetFill1X, rt.y - insetFill1Y);
	BPoint rtF3(rt.x + insetFill1X, rt.y - insetFill2Y);

	BPoint rbF0(rb.x + insetFill2X, rb.y + insetFill2Y);
	BPoint rbF1(rb.x + insetFill1X, rb.y + insetFill2Y);
	BPoint rbF2(rb.x + insetFill1X, rb.y + insetFill1Y);
	BPoint rbF3(rb.x + insetFill2X, rb.y + insetFill1Y);

	BPoint lbF0(lb.x - insetFill2X, lb.y + insetFill2Y);
	BPoint lbF1(lb.x - insetFill2X, lb.y + insetFill1Y);
	BPoint lbF2(lb.x - insetFill1X, lb.y + insetFill1Y);
	BPoint lbF3(lb.x - insetFill1X, lb.y + insetFill2Y);

	TransformObjectToView(&lt);
	TransformObjectToView(&rt);
	TransformObjectToView(&rb);
	TransformObjectToView(&lb);

	TransformObjectToView(&lt1);
	TransformObjectToView(&lt2);
	TransformObjectToView(&lt3);

	TransformObjectToView(&rt1);
	TransformObjectToView(&rt2);
	TransformObjectToView(&rt3);

	TransformObjectToView(&rb1);
	TransformObjectToView(&rb2);
	TransformObjectToView(&rb3);

	TransformObjectToView(&lb1);
	TransformObjectToView(&lb2);
	TransformObjectToView(&lb3);

	TransformObjectToView(&ltF0);
	TransformObjectToView(&ltF1);
	TransformObjectToView(&ltF2);
	TransformObjectToView(&ltF3);

	TransformObjectToView(&rtF0);
	TransformObjectToView(&rtF1);
	TransformObjectToView(&rtF2);
	TransformObjectToView(&rtF3);

	TransformObjectToView(&rbF0);
	TransformObjectToView(&rbF1);
	TransformObjectToView(&rbF2);
	TransformObjectToView(&rbF3);

	TransformObjectToView(&lbF0);
	TransformObjectToView(&lbF1);
	TransformObjectToView(&lbF2);
	TransformObjectToView(&lbF3);

	view->PushState();

	view->MovePenTo(B_ORIGIN);
	uint32 flags = view->Flags();
	view->SetFlags(flags | B_SUBPIXEL_PRECISE);

	// white corner fills
	BShape shape;
	shape.MoveTo(ltF0);
	shape.LineTo(ltF1);
	shape.LineTo(ltF2);
	shape.LineTo(ltF3);
	shape.Close();

	shape.MoveTo(rtF0);
	shape.LineTo(rtF1);
	shape.LineTo(rtF2);
	shape.LineTo(rtF3);
	shape.Close();

	shape.MoveTo(rbF0);
	shape.LineTo(rbF1);
	shape.LineTo(rbF2);
	shape.LineTo(rbF3);
	shape.Close();

	shape.MoveTo(lbF0);
	shape.LineTo(lbF1);
	shape.LineTo(lbF2);
	shape.LineTo(lbF3);
	shape.Close();

	view->SetHighColor(255, 255, 255);
	view->FillShape(&shape);

	shape.Clear();

	// transparent side fills

	shape.MoveTo(ltF0);
	shape.LineTo(lt3);
	shape.LineTo(rt1);
	shape.LineTo(rtF0);
	shape.Close();

	shape.MoveTo(rtF0);
	shape.LineTo(rt3);
	shape.LineTo(rb1);
	shape.LineTo(rbF0);
	shape.Close();

	shape.MoveTo(rbF0);
	shape.LineTo(rb3);
	shape.LineTo(lb1);
	shape.LineTo(lbF0);
	shape.Close();

	shape.MoveTo(lbF0);
	shape.LineTo(lb3);
	shape.LineTo(lt1);
	shape.LineTo(ltF0);
	shape.Close();

	view->SetHighColor(0, 0, 0, 30);
	view->SetDrawingMode(B_OP_ALPHA);
	view->FillShape(&shape);

	// white outlines

	shape.Clear();

	shape.MoveTo(ltF0);
	shape.LineTo(rtF0);
	shape.LineTo(rbF0);
	shape.LineTo(lbF0);
	shape.Close();

	view->SetHighColor(255, 255, 255, 200);
	view->StrokeShape(&shape);

	// black outlines

	shape.Clear();

	shape.MoveTo(lt1);
	shape.LineTo(rt3);
	shape.LineTo(rt2);
	shape.LineTo(rt1);
	shape.LineTo(rb3);
	shape.LineTo(rb2);
	shape.LineTo(rb1);
	shape.LineTo(lb3);
	shape.LineTo(lb2);
	shape.LineTo(lb1);
	shape.LineTo(lt3);
	shape.LineTo(lt2);
	shape.Close();

	view->SetHighColor(0, 0, 0, 200);
//	view->SetDrawingMode(B_OP_ALPHA);
	view->StrokeShape(&shape);

	view->PopState();
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
	bounds.InsetBy(-8, -8);
	return bounds;
}

// #pragma mark -

// StartTransaction
Command*
TransformToolState::StartTransaction(const char* commandName)
{
	return NULL;
}

// point_line_dist
float
point_line_dist(BPoint start, BPoint end, BPoint p, float radius)
{
	BRect r(min_c(start.x, end.x),
			min_c(start.y, end.y),
			max_c(start.x, end.x),
			max_c(start.y, end.y));
	r.InsetBy(-radius, -radius);
	if (r.Contains(p)) {
		return fabs(agg::calc_line_point_distance(start.x, start.y,
												  end.x, end.y,
												  p.x, p.y));
	}
	return min_c(point_point_distance(start, p),
				 point_point_distance(end, p));
}

// DragStateFor
TransformToolState::DragState*
TransformToolState::DragStateFor(BPoint canvasWhere, float zoomLevel) const
{
	float inset = 7.0 / zoomLevel;

	// First priority has the inside of the box, checked with some inset so
	// that the user can drag the whole box when the box is very small and
	// the click is otherwise near enough to a corner.

	BPoint where = canvasWhere;
	TransformCanvasToObject(&where);

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
		else if (dist == dRT)
			return fDragRTState;
		else if (dist == dRB)
			return fDragRBState;
		else if (dist == dLB)
			return fDragLBState;
	}

	// Next priority have the sides.

	float dL = point_line_dist(lt, lb, canvasWhere, inset);
	float dR = point_line_dist(rt, rb, canvasWhere, inset);
	float dT = point_line_dist(lt, rt, canvasWhere, inset);
	float dB = point_line_dist(lb, rb, canvasWhere, inset);
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

	// Last, check inside of the box again.
	if (fModifiedBox.Contains(where))
		return fDragBoxState;

	return NULL;
}

// SetBox
void
TransformToolState::SetBox(const BRect& box)
{
	fOriginalBox = box;
	fModifiedBox = box;
	UpdateBounds();
}

// SetModifiedBox
void
TransformToolState::SetModifiedBox(const BRect& box)
{
	fModifiedBox = box;
	UpdateBounds();
}

// LocalXScale
float
TransformToolState::LocalXScale() const
{
	if (fOriginalBox.Width() == 0.0)
		return 1.0;
	return fModifiedBox.Width() / fOriginalBox.Width();
}

// LocalYScale
float
TransformToolState::LocalYScale() const
{
	if (fOriginalBox.Height() == 0.0)
		return 1.0;
	return fModifiedBox.Height() / fOriginalBox.Height();
}

