/*
 * Copyright 2012-2013 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef PATH_TOOL_STATE_H
#define PATH_TOOL_STATE_H

#include <Messenger.h>

#include "DragStateViewState.h"
#include "HashSetHugo.h"
#include "List.h"
#include "Path.h"
#include "Selection.h"
#include "Shape.h"
#include "Style.h"

class BMessageRunner;
class BShape;
class CurrentColor;
class Document;
class Layer;

class PathToolState : public DragStateViewState,
	public Selection::Controller, public Selection::Listener,
	public Listener, public Path::Listener {
public:
								PathToolState(StateView* view,
									Document* document, Selection* selection,
									CurrentColor* color,
									const BMessenger& configView);
	virtual						~PathToolState();

	// ViewState interface
	virtual	void				Init();
	virtual	void				Cleanup();

	virtual	bool				MessageReceived(BMessage* message,
									UndoableEdit** _edit);

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

private:
			void				_DrawControls(PlatformDrawContext& drawContext);

private:
			class PlatformDelegate;

			class PickShapeState;
			class CreateShapeState;
			class DragPathPointState;
			class ToggleSmoothSharpState;
			class AddPathPointState;
			class InsertPathPointState;
			class RemovePathPointState;
			class ClosePathState;

			friend class PickShapeState;
			friend class AddPathPointState;
			friend class InsertPathPointState;
			friend class ToggleSmoothSharpState;
			friend class ClosePathState;

			enum {
				POINT		= 1 << 0,
				POINT_IN	= 1 << 1,
				POINT_OUT	= 1 << 2,

				POINT_ALL	= POINT | POINT_IN | POINT_OUT
			};

			class PathPoint {
			public:
				PathPoint()
					: fPath(NULL)
					, fIndex(-1)
					, fWhich(0)
				{
				}

				PathPoint(Path* path, int32 index, int32 which)
					: fPath(path)
					, fIndex(index)
					, fWhich(which)
				{
				}

				PathPoint(const PathPoint& other)
					: fPath(other.fPath)
					, fIndex(other.fIndex)
					, fWhich(other.fWhich)
				{
				}

				bool operator==(const PathPoint& other) const
				{
					return IsSameIndex(other) && fWhich == other.fWhich;
				}

				bool operator!=(const PathPoint& other) const
				{
					return !(*this == other);
				}

				PathPoint& operator=(const PathPoint& other)
				{
					fPath = other.fPath;
					fIndex = other.fIndex;
					fWhich = other.fWhich;
					return *this;
				}

				bool IsSameIndex(const PathPoint& other) const
				{
					return fPath == other.fPath && fIndex == other.fIndex;
				}

				Path* GetPath() const
				{
					return fPath;
				}

				int32 GetIndex() const
				{
					return fIndex;
				}

				int32 GetWhich() const
				{
					return fWhich;
				}

				bool GetPoint(BPoint& point) const
				{
					if (fPath == NULL)
						return false;
					return fPath->GetPointAt(fIndex, point);
				}

				bool GetPointIn(BPoint& point) const
				{
					if (fPath == NULL)
						return false;
					return fPath->GetPointInAt(fIndex, point);
				}

				bool GetPointOut(BPoint& point) const
				{
					if (fPath == NULL)
						return false;
					return fPath->GetPointOutAt(fIndex, point);
				}

				size_t GetHashCode() const
				{
					return (size_t)fPath ^ fIndex ^ (fWhich << 4);
				}

			private:
				Path*			fPath;
				int32			fIndex;
				int32			fWhich;
			};

			typedef HashSet<PathPoint> PointSelection;

private:
			void				_SelectPoint(const PathPoint& point,
									bool extend);
			void				_DeselectPoint(const PathPoint& point);
			void				_DeselectPoints();

			void				_SetSelectionRect(const BRect& rect);

private:
			PlatformDelegate*	fPlatformDelegate;

			PickShapeState*		fPickShapeState;
			CreateShapeState*	fCreateShapeState;
			DragPathPointState*	fDragPathPointState;
			ToggleSmoothSharpState*	fToggleSmoothSharpState;
			AddPathPointState*	fAddPathPointState;
			InsertPathPointState* fInsertPathPointState;
			RemovePathPointState* fRemovePathPointState;
			ClosePathState*		fClosePathState;

			Document*			fDocument;
			Selection*			fSelection;
			CurrentColor*		fCurrentColor;

			BMessenger			fConfigViewMessenger;

			Layer*				fInsertionLayer;
			int32				fInsertionIndex;

			Shape*				fShape;

			PathList			fPaths;
			PathRef				fCurrentPath;
	mutable	PathPoint			fHoverPathPoint;
			PointSelection		fPointSelection;
			BRect				fSelectionRect;

			StyleRef			fStyle;

			bool				fIgnoreColorColorNotifiactions;
};

#endif // PATH_TOOL_STATE_H
