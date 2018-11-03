/*
 * Copyright 2012-2015 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef PATH_TOOL_STATE_H
#define PATH_TOOL_STATE_H

#include <Messenger.h>

#include "DragStateViewState.h"
#include "List.h"
#include "PathPoint.h"
#include "Selection.h"
#include "Shape.h"
#include "Style.h"

class BMessageRunner;
class BShape;
class CurrentColor;
class Document;
class Layer;
class PathTool;

class PathToolState : public DragStateViewState,
	public Selection::Controller, public Selection::Listener,
	public Listener, public Path::Listener {
public:
								PathToolState(StateView* view, PathTool* tool,
									Document* document, Selection* selection,
									CurrentColor* color,
									const BMessenger& configView);
	virtual						~PathToolState();

	// ViewState interface
	virtual	void				Init();
	virtual	void				Cleanup();

	virtual	bool				MessageReceived(BMessage* message,
									UndoableEdit** _edit);

	// mouse tracking
	virtual	UndoableEdit*		MouseUp();

	// modifiers
	virtual	void				ModifiersChanged(uint32 modifiers);

	// TODO: mouse wheel
	virtual	bool				HandleKeyDown(const StateView::KeyEvent& event,
									UndoableEdit** _edit);
	virtual	bool				HandleKeyUp(const StateView::KeyEvent& event,
									UndoableEdit** _edit);

	virtual void				Draw(PlatformDrawContext& drawContext);

	virtual	BRect				Bounds() const;

	// DragStateViewState interface
	virtual	UndoableEdit*		StartTransaction(const char* editName);

	virtual	DragState*			DragStateFor(BPoint canvasWhere,
									float zoomLevel) const;

	// Selection::Listener interface
	virtual	void				ObjectSelected(const Selectable& object,
									const Selection::Controller* controller);
	virtual	void				ObjectDeselected(const Selectable& object,
									const Selection::Controller* controller);

	// Listener interface
	virtual	void				ObjectChanged(const Notifier* object);

public:
	// PathListener interface
	virtual	void				PointAdded(const Path* path, int32 index);
	virtual	void				PointRemoved(const Path* path, int32 index);
	virtual	void				PointChanged(const Path* path, int32 index);
	virtual	void				PathChanged(const Path* path);
	virtual	void				PathClosedChanged(const Path* path);
	virtual	void				PathReversed(const Path* path);

	// PathToolState
			void				SetInsertionInfo(Layer* layer, int32 index);
			bool				CreateShape(BPoint canvasLocation);
			bool				CreatePath(BPoint canvasLocation);

			void				SetShape(Shape* shape,
									bool modifySelection = false);

			void				AddPath(PathInstance* path);

			void				Confirm();
			void				Cancel();

private:
			void				_DrawControls(PlatformDrawContext& drawContext);

private:
			class PlatformDelegate;

			class PickShapeState;
			class SelectPointsState;
			class DragPathPointState;
			class DragSelectionState;
			class ToggleSmoothSharpState;
			class AddPathPointState;
			class InsertPathPointState;
			class RemovePathPointState;
			class ClosePathState;
			class CreateShapeState;

			friend class PickShapeState;
			friend class SelectPointsState;
			friend class DragSelectionState;
			friend class AddPathPointState;
			friend class InsertPathPointState;
			friend class ToggleSmoothSharpState;
			friend class ClosePathState;

private:
			void				_SetHoverPoint(const PathPoint& point) const;

			void				_SelectPoint(const PathPoint& point,
									bool extend);
			void				_DeselectPoint(const PathPoint& point);
			void				_DeselectPoints();

			void				_SetSelectionRect(const BRect& rect,
									const PointSelection& previousSelection);

			void				_AdoptShapePaint();
			
			bool				_NudgeSelection(float xOffset, float yOffset);

private:
			PathTool*			fTool;

			PlatformDelegate*	fPlatformDelegate;

			PickShapeState*		fPickShapeState;
			SelectPointsState*	fSelectPointsState;
			DragPathPointState*	fDragPathPointState;
			DragSelectionState*	fDragSelectionState;
			ToggleSmoothSharpState*	fToggleSmoothSharpState;
			AddPathPointState*	fAddPathPointState;
			InsertPathPointState* fInsertPathPointState;
			RemovePathPointState* fRemovePathPointState;
			ClosePathState*		fClosePathState;
			CreateShapeState*	fCreateShapeState;

			Document*			fDocument;
			Selection*			fSelection;
			CurrentColor*		fCurrentColor;

			BMessenger			fConfigViewMessenger;

			Layer*				fInsertionLayer;
			int32				fInsertionIndex;

			Shape*				fShape;

			PathList			fPaths;
			PathInstanceRef		fCurrentPath;
	mutable	PathPoint			fHoverPathPoint;
			PointSelection		fPointSelection;
			BRect				fSelectionRect;

			bool				fIgnoreColorNotifiactions;
};

#endif // PATH_TOOL_STATE_H
