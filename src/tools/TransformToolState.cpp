/*
 * Copyright 2009-2010 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "TransformToolState.h"

#include <Cursor.h>
#include <Shape.h>

#include <new>

#include <agg_math.h>

#include "ChangeAreaCommand.h"
#include "CommandStack.h"
#include "cursors.h"
#include "Document.h"
#include "Layer.h"
#include "Rect.h"
#include "Shape.h"
#include "support.h"


enum {
	MSG_OBJECT_AREA_CHANGED		= 'oacd',
	MSG_OBJECT_DELETED			= 'odlt',
};

// constructor
TransformToolState::RectLOAdapater::RectLOAdapater(BHandler* handler)
	: RectListener()
	, AbstractLOAdapter(handler)
{
}

// destructor
TransformToolState::RectLOAdapater::~RectLOAdapater()
{
}

// AreaChanged
void
TransformToolState::RectLOAdapater::AreaChanged(Rect* rect, const BRect& oldArea,
	const BRect& newArea)
{
	BMessage message(MSG_OBJECT_AREA_CHANGED);
	message.AddPointer("object", rect);
	message.AddRect("area", newArea);

	DeliverMessage(message);
}

// Deleted
void
TransformToolState::RectLOAdapater::Deleted(Rect* rect)
{
	BMessage message(MSG_OBJECT_DELETED);
	message.AddPointer("object", rect);

	DeliverMessage(message);
}

// #pragma mark -

// constructor
TransformToolState::ShapeLOAdapater::ShapeLOAdapater(BHandler* handler)
	: ShapeListener()
	, AbstractLOAdapter(handler)
{
}

// destructor
TransformToolState::ShapeLOAdapater::~ShapeLOAdapater()
{
}

// AreaChanged
void
TransformToolState::ShapeLOAdapater::AreaChanged(Shape* shape, const BRect& oldArea,
	const BRect& newArea)
{
	BMessage message(MSG_OBJECT_AREA_CHANGED);
	message.AddPointer("object", shape);
	message.AddRect("area", newArea);

	DeliverMessage(message);
}

// Deleted
void
TransformToolState::ShapeLOAdapater::Deleted(Shape* shape)
{
	BMessage message(MSG_OBJECT_DELETED);
	message.AddPointer("object", shape);

	DeliverMessage(message);
}

// #pragma mark -

class TransformToolState::DragBoxState : public DragStateViewState::DragState {
public:
	DragBoxState(TransformToolState* parent)
		: DragState(parent)
		, fParent(parent)
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
#ifdef __HAIKU__
		if (fObject != NULL)
			return BCursor(B_CURSOR_ID_FOLLOW_LINK);
#endif
		return BCursor(B_CURSOR_SYSTEM_DEFAULT);
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
		Document* document, Selection* selection)
	: DragStateViewState(view)
	, fOriginalBox(box)
	, fModifiedBox(box)

	, fPickObjectState(new PickObjectState(this))

	, fDragBoxState(new (std::nothrow) DragBoxState(this))

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

	, fRectLOAdapter(view)
	, fShapeLOAdapter(view)
{
	fSelection->AddListener(this);
}

// destructor
TransformToolState::~TransformToolState()
{
	fSelection->RemoveListener(this);

	delete fPickObjectState;
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

// MessageReceived
bool
TransformToolState::MessageReceived(BMessage* message, Command** _command)
{
	bool handled = true;

	switch (message->what) {
		case MSG_OBJECT_AREA_CHANGED: {
			Object* object;
			BRect area;
			if (message->FindPointer("object", (void**)&object) == B_OK
				&& message->FindRect("area", &area) == B_OK)
				if (object == fObject && !IsDragging())
					SetModifiedBox(area, false);
			break;
		}
		case MSG_OBJECT_DELETED: {
			Object* object;
			if (message->FindPointer("object", (void**)&object) == B_OK)
				if (object == fObject)
					SetObject(NULL);
			break;
		}

		default:
			handled = ViewState::MessageReceived(message, _command);
	}

	return handled;
}

// MouseDown
void
TransformToolState::MouseDown(BPoint where, uint32 buttons, uint32 clicks)
{
	if (!fDocument->WriteLock())
		return;

	DragStateViewState::MouseDown(where, buttons, clicks);

	fDocument->WriteUnlock();
}

// MouseMoved
void
TransformToolState::MouseMoved(BPoint where, uint32 transit,
	const BMessage* dragMessage)
{
	if (!fDocument->WriteLock())
		return;

	DragStateViewState::MouseMoved(where, transit, dragMessage);

	fDocument->WriteUnlock();
}

// MouseUp
Command*
TransformToolState::MouseUp()
{
	if (!fDocument->WriteLock())
		return NULL;

	Command* command = DragStateViewState::MouseUp();

	fDocument->WriteUnlock();

	return command;
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

	uint32 flags = view->Flags();
	bool round = true;
	if (ViewspaceRotation() != 0.0) {
		view->SetFlags(flags | B_SUBPIXEL_PRECISE);
		round = false;
	} else
		view->SetFlags(flags & ~B_SUBPIXEL_PRECISE);

	TransformObjectToView(&lt, round);
	TransformObjectToView(&rt, round);
	TransformObjectToView(&rb, round);
	TransformObjectToView(&lb, round);

	TransformObjectToView(&lt1, round);
	TransformObjectToView(&lt2, round);
	TransformObjectToView(&lt3, round);

	TransformObjectToView(&rt1, round);
	TransformObjectToView(&rt2, round);
	TransformObjectToView(&rt3, round);

	TransformObjectToView(&rb1, round);
	TransformObjectToView(&rb2, round);
	TransformObjectToView(&rb3, round);

	TransformObjectToView(&lb1, round);
	TransformObjectToView(&lb2, round);
	TransformObjectToView(&lb3, round);

	TransformObjectToView(&ltF0, round);
	TransformObjectToView(&ltF1, round);
	TransformObjectToView(&ltF2, round);
	TransformObjectToView(&ltF3, round);

	TransformObjectToView(&rtF0, round);
	TransformObjectToView(&rtF1, round);
	TransformObjectToView(&rtF2, round);
	TransformObjectToView(&rtF3, round);

	TransformObjectToView(&rbF0, round);
	TransformObjectToView(&rbF1, round);
	TransformObjectToView(&rbF2, round);
	TransformObjectToView(&rbF3, round);

	TransformObjectToView(&lbF0, round);
	TransformObjectToView(&lbF1, round);
	TransformObjectToView(&lbF2, round);
	TransformObjectToView(&lbF3, round);

	view->PushState();

	view->MovePenTo(B_ORIGIN);

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
	view->SetFlags(flags);
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
	bounds.InsetBy(-10, -10);
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
	if (object == fObject)
		SetObject(NULL);
}

// #pragma mark -

// SetObject
void
TransformToolState::SetObject(Object* object, bool modifySelection)
{
	BRect box;
	BoundedObject* boundedObject = dynamic_cast<BoundedObject*>(object);
	Rect* rect = dynamic_cast<Rect*>(object);
	Shape* shape = dynamic_cast<Shape*>(object);

	if (rect != NULL)
		box = rect->Area();
	else if (shape != NULL)
		box = shape->Area();
	else if (boundedObject != NULL)
		box = boundedObject->Bounds();

	if (object != NULL) {
		if (modifySelection)
			fSelection->Select(Selectable(boundedObject), this);
		SetObjectToCanvasTransformation(object->Transformation());
	} else {
		if (modifySelection)
			fSelection->DeselectAll(this);
		SetObjectToCanvasTransformation(Transformable());
	}

	SetTransformable(object);
	SetBox(box);
}

// SetTransformablee
void
TransformToolState::SetTransformable(Transformable* object)
{
	_UnregisterObject(fObject);

	fObject = object;

	_RegisterObject(fObject);

	if (fObject != NULL) {
		// TODO: More setup (listener...)
		fOriginalTransformation = *fObject;
	}
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
TransformToolState::SetModifiedBox(const BRect& box, bool apply)
{
	if (fModifiedBox == box)
		return;

	fModifiedBox = box;
	UpdateBounds();

	if (apply && fObject != NULL && fDocument->WriteLock()) {
		BRect area;
		area.left = min_c(fModifiedBox.left, fModifiedBox.right);
		area.top = min_c(fModifiedBox.top, fModifiedBox.bottom);
		area.right = max_c(fModifiedBox.left, fModifiedBox.right);
		area.bottom = max_c(fModifiedBox.top, fModifiedBox.bottom);

		Shape* shape = dynamic_cast<Shape*>(fObject);
		Rect* rect = dynamic_cast<Rect*>(fObject);
		BoundedObject* boundedObject = dynamic_cast<BoundedObject*>(fObject);
		if (shape != NULL) {
			Command* command = new ChangeAreaCommand<Shape>(shape, area);
			fDocument->CommandStack()->Perform(command);
		} else if (rect != NULL) {
			Command* command = new ChangeAreaCommand<Rect>(rect, area);
			fDocument->CommandStack()->Perform(command);
		} else if (boundedObject != NULL) {
			// TODO: This can't be right...
			Transformable newTransformation;
			newTransformation.TranslateBy(
				fModifiedBox.LeftTop() - fOriginalBox.LeftTop());
			newTransformation.ScaleBy(fModifiedBox.LeftTop(), LocalXScale(),
				LocalYScale());
			newTransformation.Multiply(fOriginalTransformation);
			boundedObject->SetTransformable(newTransformation);
		}

		fDocument->WriteUnlock();
	}
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

// #pragma mark -

// _RegisterObject
void
TransformToolState::_RegisterObject(Transformable* object)
{
	Shape* shape = dynamic_cast<Shape*>(object);
	Rect* rect = dynamic_cast<Rect*>(object);

	if (shape != NULL) {
		shape->AddReference();
		shape->AddListener(&fShapeLOAdapter);
	} else if (rect != NULL) {
		rect->AddReference();
		rect->AddListener(&fRectLOAdapter);
	}
}

// _UnregisterObject
void
TransformToolState::_UnregisterObject(Transformable* object)
{
	Shape* shape = dynamic_cast<Shape*>(object);
	Rect* rect = dynamic_cast<Rect*>(object);

	if (shape != NULL) {
		shape->RemoveListener(&fShapeLOAdapter);
		shape->RemoveReference();
	} else if (rect != NULL) {
		rect->RemoveListener(&fRectLOAdapter);
		rect->RemoveReference();
	}
}

