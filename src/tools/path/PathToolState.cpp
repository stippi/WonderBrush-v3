#ifndef _PATHTOOLSTATE_CPP_
#define _PATHTOOLSTATE_CPP_

/*
 * Copyright 2012-2013 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "PathToolState.h"
#include "PathToolStatePlatformDelegate.h"

#include <Cursor.h>
#include <MessageRunner.h>
#include <Shape.h>

#include <new>

#include <agg_math.h>

#include "EditManager.h"
#include "CompoundEdit.h"
#include "CurrentColor.h"
#include "cursors.h"
#include "Document.h"
#include "Layer.h"
#include "ObjectAddedEdit.h"
#include "support.h"
#include "Shape.h"
#include "StyleSetFillPaintEdit.h"
#include "TransformObjectEdit.h"
#include "ui_defines.h"

enum {
	MSG_UPDATE_BOUNDS	= 'ptub',
	MSG_SHAPE_CHANGED	= 'ptsc',
};

// PickShapeState
class PathToolState::PickShapeState : public DragStateViewState::DragState {
public:
	PickShapeState(PathToolState* parent)
		: DragState(parent)
		, fParent(parent)
		, fShape(NULL)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		// Setup tool and switch to drag left/top state
		fParent->SetShape(fShape, true);

		if (fShape == NULL)
			return;

//		fParent->SetDragState(fParent->fDragCaretState);
//		fParent->fDragCaretState->SetOrigin(origin);
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		// Never reached.
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		if (fShape != NULL)
			return BCursor(B_CURSOR_ID_FOLLOW_LINK);
		return BCursor(B_CURSOR_ID_SYSTEM_DEFAULT);
	}

	virtual const char* CommandName() const
	{
		return "Pick shape";
	}

	void SetShape(Shape* shape)
	{
		fShape = shape;
	}

private:
	PathToolState*		fParent;
	Shape*				fShape;
};

// SelectPointsState
class PathToolState::SelectPointsState : public DragStateViewState::DragState {
public:
	SelectPointsState(PathToolState* parent)
		: DragState(parent)
		, fParent(parent)
		, fPathPoint()
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		fStart = origin;
		fPreviousSelection = fParent->fPointSelection;
		if (fPathPoint.IsValid()) {
			if (fPreviousSelection.Contains(fPathPoint))
				fParent->_DeselectPoint(fPathPoint);
			else
				fParent->_SelectPoint(fPathPoint, true);
		}
		fDragDistance = 0.0;
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		if (fPathPoint.IsValid())
			return;

		fDragDistance = point_point_distance(current, fStart);

		if (fDragDistance * fParent->ZoomLevel() >= 5.0) {
			BRect selectionRect(
				std::min(fStart.x, current.x), std::min(fStart.y, current.y),
				std::max(fStart.x, current.x), std::max(fStart.y, current.y)
			);
			fParent->_SetSelectionRect(selectionRect, fPreviousSelection);
		} else {
			fParent->_SetSelectionRect(BRect(), fPreviousSelection);
		}
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		return BCursor(kPathSelectCursor);
	}

	virtual const char* CommandName() const
	{
		return "Select points";
	}

	void SetPathPoint(const PathPoint& point)
	{
		fPathPoint = point;
	}

	bool DeselectOnMouseUp() const
	{
		if (fPathPoint.IsValid())
			return false;

		return fDragDistance * fParent->ZoomLevel() < 5.0;
	}

private:
	PathToolState*		fParent;
	BPoint				fStart;
	PathPoint			fPathPoint;
	PointSelection		fPreviousSelection;
	double				fDragDistance;
};

enum {
	DRAG_MODE_NONE						= 0,
	DRAG_MODE_MOVE_POINT				= 1,
	DRAG_MODE_MOVE_POINT_IN				= 2,
	DRAG_MODE_MOVE_POINT_OUT			= 3,
	DRAG_MODE_MOVE_POINT_OUT_MIRROR_IN	= 4,
	DRAG_MODE_MOVE_SELECTION			= 5,
};

// DragPathPointState
class PathToolState::DragPathPointState : public DragStateViewState::DragState {
public:
	DragPathPointState(PathToolState* parent)
		: DragState(parent)
		, fParent(parent)
		, fPathPoint()
		, fDragMode(DRAG_MODE_NONE)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		fParent->TransformCanvasToObject(&origin);
		fOrigin = origin;
		BPoint point(origin);
		switch (fDragMode) {
			case DRAG_MODE_MOVE_POINT:
				fPathPoint.GetPoint(point);
				break;
			case DRAG_MODE_MOVE_POINT_IN:
				fPathPoint.GetPointIn(point);
				break;
			case DRAG_MODE_MOVE_POINT_OUT:
			case DRAG_MODE_MOVE_POINT_OUT_MIRROR_IN:
				fPathPoint.GetPointOut(point);
				break;
			default:
				break;
		}
		fClickOffset = origin - point;

		PathPoint pathPoint(fPathPoint.GetPath(), fPathPoint.GetIndex(),
			POINT_ALL);

		bool shift = (fParent->Modifiers() & B_SHIFT_KEY) != 0;

		if (shift && fParent->fPointSelection.Contains(pathPoint))
			fParent->_DeselectPoint(pathPoint);
		else
			fParent->_SelectPoint(pathPoint, shift);
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		fParent->TransformCanvasToObject(&current);

		current = current - fClickOffset;
		Path* path = fPathPoint.GetPath();

		switch (fDragMode) {
			case DRAG_MODE_MOVE_POINT:
				if (path != NULL)
					path->SetPoint(fPathPoint.GetIndex(), current);
				break;
			case DRAG_MODE_MOVE_POINT_IN:
				if (path != NULL)
					path->SetPointIn(fPathPoint.GetIndex(), current);
				break;
			case DRAG_MODE_MOVE_POINT_OUT:
				if (path != NULL)
					path->SetPointOut(fPathPoint.GetIndex(), current, false);
				break;
			case DRAG_MODE_MOVE_POINT_OUT_MIRROR_IN:
				if (path != NULL)
					path->SetPointOut(fPathPoint.GetIndex(), current, true);
				break;
			case DRAG_MODE_MOVE_SELECTION:
				break;
			default:
				break;
		}
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		return BCursor(kPathMoveCursor);
	}

	virtual const char* CommandName() const
	{
		return "Drag path point";
	}

	void SetPathPoint(const PathPoint& point)
	{
		fPathPoint = point;
	}

	void SetDragMode(uint32 dragMode)
	{
		fDragMode = dragMode;
	}

private:
	PathToolState*		fParent;
	PathPoint			fPathPoint;
	uint32				fDragMode;
	BPoint				fClickOffset;
};

// DragSelectionState
class PathToolState::DragSelectionState : public DragStateViewState::DragState {
public:
	DragSelectionState(PathToolState* parent)
		: DragState(parent)
		, fParent(parent)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		fParent->TransformCanvasToObject(&origin);
		fOrigin = origin;
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		fParent->TransformCanvasToObject(&current);
		BPoint offset = current - fOrigin;
		fOrigin = current;

		// TODO: Stop Path notifications until all points are modified

		PointSelection::Iterator iterator
			= fParent->fPointSelection.GetIterator();
		while (iterator.HasNext()) {
			PathPoint pathPoint = iterator.Next();
			pathPoint.OffsetBy(offset);
		}
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		return BCursor(kPathMoveCursor);
	}

	virtual const char* CommandName() const
	{
		return "Drag path points";
	}

private:
	PathToolState*		fParent;
};

// ToggleSmoothSharpState
class PathToolState::ToggleSmoothSharpState
	: public DragStateViewState::DragState {
public:
	ToggleSmoothSharpState(PathToolState* parent)
		: DragState(parent)
		, fParent(parent)
		, fPathPoint()
		, fDragMode(DRAG_MODE_NONE)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		fParent->TransformCanvasToObject(&origin);
		fOrigin = origin;

		Path* path = fPathPoint.GetPath();
		int32 index = fPathPoint.GetIndex();
		BPoint point;
		BPoint pointIn;
		BPoint pointOut;
		if (fPathPoint.GetPoint(point)
			&& fPathPoint.GetPointIn(pointIn)
			&& fPathPoint.GetPointOut(pointOut)) {
			switch (fDragMode) {
				case DRAG_MODE_MOVE_POINT_IN:
					path->SetPoint(index, point, pointIn, pointOut, false);
					fParent->fDragPathPointState->SetPathPoint(PathPoint(path,
						index, POINT_IN));
					break;
				case DRAG_MODE_MOVE_POINT_OUT:
					path->SetPoint(index, point, pointIn, pointOut, false);
					fParent->fDragPathPointState->SetPathPoint(PathPoint(path,
						index, POINT_OUT));
					break;
				case DRAG_MODE_MOVE_POINT_OUT_MIRROR_IN:
					path->SetPoint(index, point, point, point, true);
					fParent->fDragPathPointState->SetPathPoint(PathPoint(path,
						index, POINT_OUT));
					break;
			}
			fParent->fDragPathPointState->SetDragMode(fDragMode);
			fParent->_SelectPoint(PathPoint(path, index, POINT_ALL), false);
		} else {
			fParent->fDragPathPointState->SetPathPoint(PathPoint());
		}
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		BPoint originalCurrent(current);
		fParent->TransformCanvasToObject(&current);

		double dragDistance = point_point_distance(fOrigin, current);
		if (dragDistance * fParent->ZoomLevel() > 7.0) {
			BPoint originalOrigin(fOrigin);
			fParent->TransformObjectToCanvas(&originalOrigin);

			fParent->SetDragState(fParent->fDragPathPointState);
			fParent->fDragPathPointState->SetOrigin(originalOrigin);
			fParent->fDragPathPointState->DragTo(originalCurrent, modifiers);
		}
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		return BCursor(kPathSharpCursor);
	}

	virtual const char* CommandName() const
	{
		return "Set path point sharp";
	}

	void SetPathPoint(const PathPoint& point)
	{
		fPathPoint = point;
	}

	void SetDragMode(uint32 dragMode)
	{
		fDragMode = dragMode;
	}

private:
	PathToolState*		fParent;
	PathPoint			fPathPoint;
	uint32				fDragMode;
};

// AddPathPointState
class PathToolState::AddPathPointState : public DragStateViewState::DragState {
public:
	AddPathPointState(PathToolState* parent)
		: DragState(parent)
		, fParent(parent)
		, fPathRef()
		, fPointAdded(false)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		fParent->TransformCanvasToObject(&origin);

		fOrigin = origin;
		Path* path = fPathRef.Get();
		if (path != NULL && path->AddPoint(origin)) {
			fPointAdded = true;
			fParent->_SelectPoint(PathPoint(path, path->CountPoints() - 1,
				POINT_ALL), false);
		}
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		BPoint originalCurrent(current);
		fParent->TransformCanvasToObject(&current);

		double dragDistance = point_point_distance(fOrigin, current);
		if (fPointAdded && dragDistance * fParent->ZoomLevel() > 7.0) {
			Path* path = fPathRef.Get();
			fParent->fDragPathPointState->SetPathPoint(PathPoint(path,
				path->CountPoints() - 1, POINT_OUT));
			fParent->fDragPathPointState->SetDragMode(
				DRAG_MODE_MOVE_POINT_OUT_MIRROR_IN);

			BPoint originalOrigin(fOrigin);
			fParent->TransformObjectToCanvas(&originalOrigin);

			fParent->SetDragState(fParent->fDragPathPointState);
			fParent->fDragPathPointState->SetOrigin(originalOrigin);
			fParent->fDragPathPointState->DragTo(originalCurrent, modifiers);
		}
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		return BCursor(kPathAddCursor);
	}

	virtual const char* CommandName() const
	{
		return "Add path point";
	}

	void SetPath(const PathRef& pathRef)
	{
		fPathRef = pathRef;
	}

private:
	PathToolState*		fParent;
	PathRef				fPathRef;
	bool				fPointAdded;
};

// InsertPathPointState
class PathToolState::InsertPathPointState
	: public DragStateViewState::DragState {
public:
	InsertPathPointState(PathToolState* parent)
		: DragState(parent)
		, fParent(parent)
		, fPathRef()
		, fPointAdded(false)
		, fDragInsertPosition(true)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		fParent->TransformCanvasToObject(&origin);

		fOrigin = origin;
		Path* path = fPathRef.Get();
		if (path != NULL
			&& path->GetPointsAt(fIndex - 1,
				fPrevious[0], fPrevious[1], fPrevious[2], &fPreviousConnected)
			&& path->GetPointsAt(fIndex,
				fNext[0], fNext[1], fNext[2], &fNextConnected)
			&& _InsertPoint(path, origin, fIndex)) {
			fPointAdded = true;
			fParent->_SelectPoint(PathPoint(path, fIndex,
				POINT_ALL), false);
		}
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
		fParent->TransformCanvasToObject(&current);

		double dragDistance = point_point_distance(fOrigin, current);
		if (fPointAdded && dragDistance > 7.0) {
			Path* path = fPathRef.Get();
			if (fDragInsertPosition) {
				// Undo insertion and insert again at current pointer
				// location
				path->RemovePoint(fIndex);
				path->SetPoint(fIndex - 1,
					fPrevious[0], fPrevious[1], fPrevious[2],
					fPreviousConnected);
				path->SetPoint(fIndex,
					fNext[0], fNext[1], fNext[2], fNextConnected);
				_InsertPoint(path, current, fIndex);
			} else {
				// Switch to DragState for dragging the inserted point
				fParent->fDragPathPointState->SetPathPoint(PathPoint(path,
					fIndex, POINT_ALL));
				fParent->fDragPathPointState->SetDragMode(
					DRAG_MODE_MOVE_POINT);

				fParent->SetDragState(fParent->fDragPathPointState);
				fParent->fDragPathPointState->SetOrigin(fOrigin);
				fParent->fDragPathPointState->DragTo(current, modifiers);
			}
		}
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		return BCursor(kPathInsertCursor);
	}

	virtual const char* CommandName() const
	{
		return "Insert path point";
	}

	void SetPath(const PathRef& pathRef)
	{
		fPathRef = pathRef;
	}

	void SetInsertIndex(int32 index)
	{
		fIndex = index;
	}

	void SetDragInsertPosition(bool dragInsertPosition)
	{
		fDragInsertPosition = dragInsertPosition;
	}

private:
	BPoint _ScalePoint(BPoint a, BPoint b, double scale) const
	{
		return BPoint(
			a.x + (b.x - a.x) * scale,
			a.y + (b.y - a.y) * scale
		);
	}

	bool _InsertPoint(Path* path, BPoint where, int32 index)
	{
		double scale;

		BPoint point;
		BPoint pointIn;
		BPoint pointOut;

		BPoint previous;
		BPoint previousOut;
		BPoint next;
		BPoint nextIn;

		if (path->FindBezierScale(index - 1, where, &scale)
			&& scale >= 0.0 && scale <= 1.0
			&& path->GetPoint(index - 1, scale, point)) {

			path->GetPointAt(index - 1, previous);
			path->GetPointOutAt(index - 1, previousOut);
			path->GetPointAt(index, next);
			path->GetPointInAt(index, nextIn);

			where = _ScalePoint(previousOut, nextIn, scale);

			previousOut = _ScalePoint(previous, previousOut, scale);
			nextIn = _ScalePoint(next, nextIn, 1 - scale);
			pointIn = _ScalePoint(previousOut, where, scale);
			pointOut = _ScalePoint(nextIn, where, 1 - scale);

			if (path->AddPoint(point, index)) {
				path->SetPointIn(index, pointIn);
				path->SetPointOut(index, pointOut);

//				delete fInsertPointCommand;
//				fInsertPointCommand = new InsertPointCommand(path, index,
//					fSelection->Items(), fSelection->CountItems());

				path->SetPointOut(index - 1, previousOut);
				path->SetPointIn(index + 1, nextIn);

//				fCurrentPathPoint = index;
//				_ShiftSelection(fCurrentPathPoint, 1);
//				_Select(fCurrentPathPoint, fShiftDown);

				return true;
			}
		}

		return false;
	}


private:
	PathToolState*		fParent;
	PathRef				fPathRef;
	int32				fIndex;
	bool				fPointAdded;
	bool				fDragInsertPosition;

	BPoint				fPrevious[3];
	bool				fPreviousConnected;
	BPoint				fNext[3];
	bool				fNextConnected;
};

// RemovePathPointState
class PathToolState::RemovePathPointState
	: public DragStateViewState::DragState {
public:
	RemovePathPointState(PathToolState* parent)
		: DragState(parent)
		, fParent(parent)
		, fPathPoint()
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		Path* path = fPathPoint.GetPath();
		if (path == NULL)
			return;

		BPoint point;
		if (!fPathPoint.GetPoint(point))
			return;

		int32 index = fPathPoint.GetIndex();
		switch (fPathPoint.GetWhich()) {
			case POINT_IN:
				path->SetPointIn(index, point);
				break;
			case POINT_OUT:
				path->SetPointOut(index, point);
				break;
			case POINT:
				path->RemovePoint(index);
				break;
		}
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		return BCursor(kPathRemoveCursor);
	}

	virtual const char* CommandName() const
	{
		return "Remove path point";
	}

	void SetPathPoint(const PathPoint& point)
	{
		fPathPoint = point;
	}

private:
	PathToolState*		fParent;
	PathPoint			fPathPoint;
};

// ClosePathState
class PathToolState::ClosePathState : public DragStateViewState::DragState {
public:
	ClosePathState(PathToolState* parent)
		: DragState(parent)
		, fParent(parent)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		Path* path = fPathRef.Get();
		if (path != NULL)
			path->SetClosed(true);
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		return BCursor(kPathCloseCursor);
	}

	virtual const char* CommandName() const
	{
		return "Close path";
	}

	void SetPath(const PathRef& pathRef)
	{
		fPathRef = pathRef;
	}

private:
	PathToolState*		fParent;
	PathRef				fPathRef;
};

// CreateShapeState
class PathToolState::CreateShapeState : public DragStateViewState::DragState {
public:
	CreateShapeState(PathToolState* parent)
		: DragState(parent)
		, fParent(parent)
	{
	}

	virtual void SetOrigin(BPoint origin)
	{
		BPoint originalOrigin(origin);

		// Create shape
		if (!fParent->CreateShape(origin))
			return;

		fParent->TransformCanvasToObject(&origin);

		// Create path with initial point
		if (!fParent->CreatePath(origin))
			return;

		// Switch to drag path point state
		PathRef pathRef = fParent->fPaths.LastItem();
		fParent->fDragPathPointState->SetPathPoint(
			PathPoint(pathRef.Get(), 0, POINT_OUT));
		fParent->fDragPathPointState->SetDragMode(
			DRAG_MODE_MOVE_POINT_OUT_MIRROR_IN);
		fParent->SetDragState(fParent->fDragPathPointState);
		fParent->fDragPathPointState->SetOrigin(originalOrigin);
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		return BCursor(kPathNewCursor);
	}

	virtual const char* CommandName() const
	{
		return "Create shape";
	}

private:
	PathToolState*		fParent;
};

// #pragma mark -

// constructor
PathToolState::PathToolState(StateView* view, Document* document,
		Selection* selection, CurrentColor* color,
		const BMessenger& configView)
	: DragStateViewState(view)

	, fPlatformDelegate(new PlatformDelegate(this))

	, fPickShapeState(new(std::nothrow) PickShapeState(this))
	, fSelectPointsState(new(std::nothrow) SelectPointsState(this))
	, fDragPathPointState(new(std::nothrow) DragPathPointState(this))
	, fDragSelectionState(new(std::nothrow) DragSelectionState(this))
	, fToggleSmoothSharpState(new(std::nothrow) ToggleSmoothSharpState(this))
	, fAddPathPointState(new(std::nothrow) AddPathPointState(this))
	, fInsertPathPointState(new(std::nothrow) InsertPathPointState(this))
	, fRemovePathPointState(new(std::nothrow) RemovePathPointState(this))
	, fClosePathState(new(std::nothrow) ClosePathState(this))
	, fCreateShapeState(new(std::nothrow) CreateShapeState(this))

	, fDocument(document)
	, fSelection(selection)
	, fCurrentColor(color)

	, fConfigViewMessenger(configView)

	, fInsertionLayer(NULL)
	, fInsertionIndex(-1)

	, fShape(NULL)

	, fPaths()
	, fCurrentPath()

	, fIgnoreColorNotifiactions(false)
{
	// TODO: Find a way to change this later...
	SetInsertionInfo(fDocument->RootLayer(),
		fDocument->RootLayer()->CountObjects());

	fCurrentColor->AddListener(this);
}

// destructor
PathToolState::~PathToolState()
{
	SetShape(NULL);
	fCurrentColor->RemoveListener(this);
	fSelection->RemoveListener(this);

	delete fPickShapeState;
	delete fSelectPointsState;
	delete fDragPathPointState;
	delete fDragSelectionState;
	delete fToggleSmoothSharpState;
	delete fAddPathPointState;
	delete fInsertPathPointState;
	delete fRemovePathPointState;
	delete fClosePathState;
	delete fCreateShapeState;

	SetInsertionInfo(NULL, -1);

	delete fPlatformDelegate;

	for (int32 i = fPaths.CountItems() - 1; i >= 0; i--)
		fPaths.ItemAtFast(i)->RemoveListener(this);
}

// Init
void
PathToolState::Init()
{
	if (!fSelection->IsEmpty())
		ObjectSelected(fSelection->SelectableAt(0), NULL);
	fSelection->AddListener(this);
	DragStateViewState::Init();
}

// Cleanup
void
PathToolState::Cleanup()
{
	SetShape(NULL);
	DragStateViewState::Cleanup();
	fSelection->RemoveListener(this);
}

// MessageReceived
bool
PathToolState::MessageReceived(BMessage* message, UndoableEdit** _edit)
{
	bool handled = true;

	switch (message->what) {
		case MSG_UPDATE_BOUNDS:
			UpdateBounds();
			handled = true;
			break;
		case MSG_SHAPE_CHANGED:
			if (fShape != NULL) {
				SetObjectToCanvasTransformation(fShape->Transformation());
				UpdateBounds();
				UpdateDragState();
				_AdoptShapePaint();
			}
			break;
		default:
			handled = DragStateViewState::MessageReceived(message, _edit);
	}

	return handled;
}

// #pragma mark -

// MouseUp
UndoableEdit*
PathToolState::MouseUp()
{
	if (CurrentState() == fSelectPointsState) {
		_SetSelectionRect(BRect(), fPointSelection);
		if (fSelectPointsState->DeselectOnMouseUp())
			_DeselectPoints();
	}
	return DragStateViewState::MouseUp();
}

// #pragma mark -

// ModifiersChanged
void
PathToolState::ModifiersChanged(uint32 modifiers)
{
	DragStateViewState::ModifiersChanged(modifiers);
	UpdateDragState();
}

// HandleKeyDown
bool
PathToolState::HandleKeyDown(const StateView::KeyEvent& event,
	UndoableEdit** _edit)
{
	bool handled = false;

	if (fShape != NULL) {
//		bool select = (event.modifiers & B_SHIFT_KEY) != 0;

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
PathToolState::HandleKeyUp(const StateView::KeyEvent& event,
	UndoableEdit** _edit)
{
	return DragStateViewState::HandleKeyUp(event, _edit);
}

// #pragma mark -

// Draw
void
PathToolState::Draw(PlatformDrawContext& drawContext)
{
	_DrawControls(drawContext);
}

// Bounds
BRect
PathToolState::Bounds() const
{
	BRect bounds(LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN);

	class BoundsIterator : public Path::Iterator {
	public:
		BoundsIterator(BRect& bounds)
			: fBounds(bounds)
		{
		}

		virtual void MoveTo(BPoint point)
		{
			_Include(point);
		}

		virtual void LineTo(BPoint point)
		{
			_Include(point);
		}

	private:
		void _Include(const BPoint& point)
		{
			if (fBounds.left > point.x)
				fBounds.left = point.x;
			if (fBounds.right < point.x)
				fBounds.right = point.x;
			if (fBounds.top > point.y)
				fBounds.top = point.y;
			if (fBounds.bottom < point.y)
				fBounds.bottom = point.y;
		}

	private:
		BRect&	fBounds;
	};

	BoundsIterator iterator(bounds);

	for (int32 i = 0; i < fPaths.CountItems(); i++) {
		Path* path = fPaths.ItemAtFast(i).Get();
		path->Iterate(&iterator, 1.0);
		bounds = bounds | path->ControlPointBounds();
	}

	TransformObjectToView(&bounds);
	bounds.InsetBy(-15, -15);

	// Selection rect
	if (fSelectionRect.IsValid()) {
		BRect selectionRect(fSelectionRect);
		TransformCanvasToView(&selectionRect);

		bounds = bounds | selectionRect;
	}

	return bounds;
}

// #pragma mark -

// StartTransaction
UndoableEdit*
PathToolState::StartTransaction(const char* editName)
{
	return NULL;
}

// DragStateFor
PathToolState::DragState*
PathToolState::DragStateFor(BPoint canvasWhere, float zoomLevel) const
{
	BPoint objectWhere = canvasWhere;
	TransformCanvasToObject(&objectWhere);

	double inset = 10.0 / zoomLevel;

	double closestDistance = LONG_MAX;
	PathPoint closestPathPoint;

	for (int32 i = fPaths.CountItems() - 1; i >= 0; i--) {
		Path* path = fPaths.ItemAtFast(i).Get();
		for (int32 j = path->CountPoints() - 1; j >= 0; j--) {
			BPoint point;
			BPoint pointIn;
			BPoint pointOut;
			if (path->GetPointsAt(j, point, pointIn, pointOut)) {
				double distance = point_point_distance(point,
					objectWhere);
				double distanceIn = point_point_distance(pointIn,
					objectWhere);
				double distanceOut = point_point_distance(pointOut,
					objectWhere);

				if (distance < inset
					&& distance < closestDistance
					&& distance <= distanceIn
					&& distance <= distanceOut) {
					closestPathPoint = PathPoint(path, j, POINT);
					closestDistance = distance;
				}

				// Hit test in/out control points only if they are different
				// from the main control point. If in and out have the same
				// position, prefer dragging out.
				if (pointIn != point
					&& distanceIn < inset
					&& distanceIn < closestDistance
					&& distanceIn < distance
					&& distanceIn < distanceOut) {
					closestPathPoint = PathPoint(path, j, POINT_IN);
					closestDistance = distanceIn;
				} else if (pointOut != point
					&& distanceOut < inset
					&& distanceOut < closestDistance
					&& distanceOut < distance) {
					closestPathPoint = PathPoint(path, j, POINT_OUT);
					closestDistance = distanceOut;
				}
			}
		}
	}

	_SetHoverPoint(closestPathPoint);
	if (fHoverPathPoint != closestPathPoint) {
		fHoverPathPoint = closestPathPoint;
		Invalidate();
	}

	if (closestDistance < inset) {
		// Pointer over a path point
		if (closestPathPoint.GetWhich() == POINT
			&& !closestPathPoint.GetPath()->IsClosed()
			&& closestPathPoint.GetIndex() == 0
			&& closestPathPoint.GetPath()->CountPoints() > 1) {
			// Pointer over first path point of open path
			fClosePathState->SetPath(PathRef(closestPathPoint.GetPath()));
			return fClosePathState;
		}

		if ((Modifiers() & B_CONTROL_KEY) != 0) {
			// Pointer over point and control/alt key pressed
			fRemovePathPointState->SetPathPoint(closestPathPoint);
			return fRemovePathPointState;
		}

		if ((Modifiers() & B_COMMAND_KEY) != 0) {
			// Pointer over point and alt/control key pressed
			fToggleSmoothSharpState->SetPathPoint(closestPathPoint);
			switch (closestPathPoint.GetWhich()) {
				case POINT:
					fToggleSmoothSharpState->SetDragMode(
						DRAG_MODE_MOVE_POINT_OUT_MIRROR_IN);
					break;
				case POINT_IN:
					fToggleSmoothSharpState->SetDragMode(
						DRAG_MODE_MOVE_POINT_IN);
					break;
				case POINT_OUT:
					fToggleSmoothSharpState->SetDragMode(
						DRAG_MODE_MOVE_POINT_OUT);
					break;
			}
			return fToggleSmoothSharpState;
		}

		if ((Modifiers() & B_SHIFT_KEY) != 0) {
			PathPoint pathPoint(closestPathPoint.GetPath(),
				closestPathPoint.GetIndex(), POINT_ALL);
			_SetHoverPoint(pathPoint);
			fSelectPointsState->SetPathPoint(pathPoint);
			return fSelectPointsState;
		}

		if (closestPathPoint.GetWhich() == POINT) {
			PathPoint pathPoint(closestPathPoint.GetPath(),
				closestPathPoint.GetIndex(), POINT_ALL);
			if (fPointSelection.Contains(pathPoint)
				&& fPointSelection.Size() > 1) {
				// Hovering an already selected point above the main
				// path point and there are other selected points. Drag
				// them as a group
				return fDragSelectionState;
			}
		}

		fDragPathPointState->SetPathPoint(closestPathPoint);
		switch (closestPathPoint.GetWhich()) {
			case POINT:
				fDragPathPointState->SetDragMode(DRAG_MODE_MOVE_POINT);
				break;
			case POINT_IN:
				fDragPathPointState->SetDragMode(DRAG_MODE_MOVE_POINT_IN);
				break;
			case POINT_OUT:
				fDragPathPointState->SetDragMode(DRAG_MODE_MOVE_POINT_OUT);
				break;
		}
		return fDragPathPointState;
	}

	if (fPaths.CountItems() > 0 && (Modifiers() & B_SHIFT_KEY) != 0) {
		fSelectPointsState->SetPathPoint(PathPoint());
		return fSelectPointsState;
	}

	// Check if pointer is above a path segment
	inset = 7.0 / zoomLevel;
	closestDistance = LONG_MAX;
	for (int32 i = fPaths.CountItems() - 1; i >= 0; i--) {
		Path* path = fPaths.ItemAtFast(i).Get();

		float distance;
		int32 index;
		if (!path->GetDistance(objectWhere, &distance, &index))
			continue;

		if (distance < closestDistance && distance < inset) {
			closestDistance = distance;
			closestPathPoint = PathPoint(path, index, POINT);
		}
	}

	if (closestDistance < inset) {
		fInsertPathPointState->SetPath(PathRef(closestPathPoint.GetPath()));
		fInsertPathPointState->SetInsertIndex(closestPathPoint.GetIndex());
		return fInsertPathPointState;
	}

	if ((Modifiers() & B_COMMAND_KEY) != 0) {
		// If there is still no state, switch to the PickObjectsState
		// and try to find an object. If nothing is picked, unset on mouse down.
		Object* pickedObject = NULL;
		fDocument->RootLayer()->HitTest(canvasWhere, NULL, &pickedObject, true);

		Shape* pickedShape = dynamic_cast<Shape*>(pickedObject);
		if (pickedShape != NULL && pickedShape != fShape) {
			fPickShapeState->SetShape(pickedShape);
			return fPickShapeState;
		}
	}

	if (fCurrentPath.Get() != NULL) {
		if (!fCurrentPath->IsClosed()) {
			fAddPathPointState->SetPath(fCurrentPath);
			return fAddPathPointState;
		} else {
			fSelectPointsState->SetPathPoint(PathPoint());
			return fSelectPointsState;
		}
	}

	return fCreateShapeState;
}

// #pragma mark -

// ObjectSelected
void
PathToolState::ObjectSelected(const Selectable& selectable,
	const Selection::Controller* controller)
{
	if (controller == this) {
		// ignore changes triggered by ourself
		return;
	}

	Shape* shape = dynamic_cast<Shape*>(selectable.Get());
	SetShape(shape);
}

// ObjectDeselected
void
PathToolState::ObjectDeselected(const Selectable& selectable,
	const Selection::Controller* controller)
{
	if (controller == this) {
		// ignore changes triggered by ourself
		return;
	}

	Shape* shape = dynamic_cast<Shape*>(selectable.Get());
	if (shape == fShape)
		SetShape(NULL);
}

// #pragma mark -

// ObjectChanged
void
PathToolState::ObjectChanged(const Notifier* object)
{
	if (dynamic_cast<const Path*>(object) != NULL) {
		// Update asynchronously, since the notification may arrive on
		// the wrong thread.
		View()->PostMessage(MSG_UPDATE_BOUNDS);
	}

	if (fShape != NULL && object == fShape && !IsDragging()) {
		View()->PostMessage(MSG_SHAPE_CHANGED);
	}

	if (object == fCurrentColor && !fIgnoreColorNotifiactions) {
		AutoWriteLocker _(View()->Locker());

		if (fShape != NULL) {
			Style* style = fShape->Style();
			View()->PerformEdit(new(std::nothrow) StyleSetFillPaintEdit(style,
				Paint(fCurrentColor->Color())));
		}
	}
}

// #pragma mark -

// PointAdded
void
PathToolState::PointAdded(const Path* path, int32 index)
{
	_SelectPoint(PathPoint(const_cast<Path*>(path), index, POINT_ALL), false);
	ObjectChanged(path);
}

// PointRemoved
void
PathToolState::PointRemoved(const Path* path, int32 index)
{
	_DeselectPoint(PathPoint(const_cast<Path*>(path), index, POINT_ALL));
	ObjectChanged(path);
}

// PointChanged
void
PathToolState::PointChanged(const Path* path, int32 index)
{
	ObjectChanged(path);
}

// PathChanged
void
PathToolState::PathChanged(const Path* path)
{
	ObjectChanged(path);
}

// PathClosedChanged
void
PathToolState::PathClosedChanged(const Path* path)
{
	ObjectChanged(path);
}

// PathReversed
void
PathToolState::PathReversed(const Path* path)
{
//	// reverse selection along with path
//	int32 count = fSelection->CountItems();
//	int32 pointCount = fPath->CountPoints();
//	if (count > 0) {
//		Selection temp;
//		for (int32 i = 0; i < count; i++) {
//			temp.Add((pointCount - 1) - fSelection->IndexAt(i));
//		}
//		*fSelection = temp;
//	}

	ObjectChanged(path);
}

// #pragma mark -

// SetInsertionInfo
void
PathToolState::SetInsertionInfo(Layer* layer, int32 index)
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

// CreateShape
bool
PathToolState::CreateShape(BPoint canvasLocation)
{
	if (fInsertionLayer == NULL) {
		fprintf(stderr, "PathToolState::MouseDown(): No insertion layer "
			"specified\n");
		return false;
	}

	Shape* shape = new(std::nothrow) Shape();
	Style* style = new(std::nothrow) Style();
	if (shape == NULL || style == NULL) {
		fprintf(stderr, "PathToolState::CreateShape(): Failed to allocate "
			"Shape or Style. Out of memory\n");
		delete shape;
		delete style;
		return false;
	}

	style->SetFillPaint(Paint(fCurrentColor->Color()));
	shape->SetStyle(style);
	style->RemoveReference();

	shape->TranslateBy(canvasLocation);

	if (fInsertionIndex < 0)
		fInsertionIndex = 0;
	if (fInsertionIndex > fInsertionLayer->CountObjects())
		fInsertionIndex = fInsertionLayer->CountObjects();

	if (!fInsertionLayer->AddObject(shape, fInsertionIndex)) {
		fprintf(stderr, "PathToolState::CreateShape(): Failed to add "
			"Shape to Layer. Out of memory\n");
		shape->RemoveReference();
		return false;
	}

	fInsertionIndex++;

	SetShape(shape, true);

	// Our reference to this object was transferred to the Layer

	View()->PerformEdit(new(std::nothrow) ObjectAddedEdit(shape,
		fSelection));


	return true;
}

// CreatePath
bool
PathToolState::CreatePath(BPoint canvasLocation)
{
	Path* path = new(std::nothrow) Path();
	if (path == NULL) {
		fprintf(stderr, "PathToolState::CreatePath(): Failed to allocate "
			"Path. Out of memory\n");
		return false;
	}

	PathRef pathRef(path, true);

	if (!path->AddPoint(canvasLocation)) {
		fprintf(stderr, "PathToolState::CreatePath(): Failed to add "
			"path point. Out of memory\n");
		return false;
	}

	if (!path->AddListener(this)) {
		fprintf(stderr, "PathToolState::CreatePath(): Failed to add "
			"listener to path. Out of memory\n");
		return false;
	}

	if (!fPaths.Add(pathRef)) {
		fprintf(stderr, "PathToolState::CreatePath(): Failed to add "
			"path to list. Out of memory\n");
		return false;
	}

	fCurrentPath.SetTo(path);

	if (fShape != NULL)
		fShape->AddPath(pathRef);

//	View()->PerformEdit(new(std::nothrow) ObjectAddedEdit(shape,
//		fSelection));

	UpdateBounds();

	return true;
}


// SetShape
void
PathToolState::SetShape(Shape* shape, bool modifySelection)
{
	if (fShape == shape)
		return;

	for (int32 i = fPaths.CountItems() - 1; i >= 0; i--)
		fPaths.ItemAtFast(i)->RemoveListener(this);
	fPaths.Clear();

	if (fShape != NULL) {
		fShape->RemoveListener(this);
		fShape->RemoveReference();
	}

	fShape = shape;

	if (fShape != NULL) {
		fShape->AddReference();
		fShape->AddListener(this);
	}

	if (shape != NULL) {
		if (modifySelection)
			fSelection->Select(Selectable(shape), this);

		fPaths = shape->Paths();
		for (int32 i = fPaths.CountItems() - 1; i >= 0; i--)
			fPaths.ItemAtFast(i)->AddListener(this);

		fCurrentPath = fPaths.ItemAt(0);

		SetObjectToCanvasTransformation(fShape->Transformation());
//		ObjectChanged(shape);
		ObjectChanged(fCurrentPath.Get());

		_AdoptShapePaint();
	} else {
		if (modifySelection)
			fSelection->DeselectAll(this);
//		SetObjectToCanvasTransformation(Transformable());
	}
}

// #pragma mark - private

// _DrawControls
void
PathToolState::_DrawControls(PlatformDrawContext& drawContext)
{
	for (int32 i = 0; i < fPaths.CountItems(); i++) {
		Path* path = fPaths.ItemAtFast(i).Get();
		fPlatformDelegate->DrawPath(path, drawContext, *this, fPointSelection,
			fHoverPathPoint, fView->ZoomLevel());
	}

	if (fSelectionRect.IsValid()) {
		fPlatformDelegate->DrawSelectionRect(fSelectionRect, drawContext,
			*this);
	}

//	double scaleX;
//	double scaleY;
//	if (!EffectiveTransformation().GetAffineParameters(NULL, NULL, NULL,
//		&scaleX, &scaleY, NULL, NULL)) {
//		return;
//	}
//
//	scaleX *= fView->ZoomLevel();
//	scaleY *= fView->ZoomLevel();
//
//	BPoint origin(0.0f, -10.0f / scaleY);
//	TransformObjectToView(&origin, true);
//
//	BPoint widthOffset(0.0f, -10.0f / scaleY);
//	TransformObjectToView(&widthOffset, true);
//
//	fPlatformDelegate->DrawControls(drawContext, origin, widthOffset);
}

// 	_SetHoverPoint
void
PathToolState::_SetHoverPoint(const PathPoint& point) const
{
	if (fHoverPathPoint != point) {
		fHoverPathPoint = point;
		Invalidate();
	}
}

// _SetSelectionRect
void
PathToolState::_SetSelectionRect(const BRect& rect,
	const PointSelection& previousSelection)
{
	if (fSelectionRect == rect)
		return;
	fSelectionRect = rect;
	if (rect.IsValid()) {
		PointSelection selection(previousSelection);
		for (int32 i = fPaths.CountItems() - 1; i >= 0; i--) {
			Path* path = fPaths.ItemAtFast(i).Get();
			for (int32 j = path->CountPoints() - 1; j >= 0; j--) {
				BPoint point;
				BPoint pointIn;
				BPoint pointOut;
				if (!path->GetPointsAt(j, point, pointIn, pointOut))
					continue;
				TransformObjectToCanvas(&point);
				TransformObjectToCanvas(&pointIn);
				TransformObjectToCanvas(&pointOut);

				PathPoint pathPoint(path, j, POINT_ALL);

				if (rect.Contains(point) || rect.Contains(pointIn)
					|| rect.Contains(pointOut)) {
					// Path point within selection rect
					if (previousSelection.Contains(pathPoint))
						selection.Remove(pathPoint);
					else
						selection.Add(pathPoint);
				}
			}
		}
		fPointSelection = selection;
	}
	UpdateBounds();
}

// _SelectPoint
void
PathToolState::_SelectPoint(const PathToolState::PathPoint& point, bool extend)
{
	if (!extend)
		_DeselectPoints();
	else if (fPointSelection.Contains(point))
		return;

	fPointSelection.Add(point);

	Invalidate();
}

// _DeselectPoint
void
PathToolState::_DeselectPoint(const PathToolState::PathPoint& point)
{
	if (!fPointSelection.Contains(point))
		return;

	fPointSelection.Remove(point);

	Invalidate();
}

// _DeselectPoints
void
PathToolState::_DeselectPoints()
{
	if (fPointSelection.Size() == 0)
		return;

	fPointSelection.Clear();

	Invalidate();
}

// _AdoptShapePaint
void
PathToolState::_AdoptShapePaint()
{
	Style* style = fShape->Style();
	Paint* fillPaint = style->FillPaint();
	if (fillPaint->Type() == Paint::COLOR) {
		fIgnoreColorNotifiactions = true;
		fCurrentColor->SetColor(fillPaint->Color());
		fIgnoreColorNotifiactions = false;
	}
}

#endif	// _PATHTOOLSTATE_CPP_
