/*
 * Copyright 2006-2007, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */

#ifndef TRANSFORM_BOX_H
#define TRANSFORM_BOX_H

#include <List.h>

#include "ChannelTransform.h"
#include "ViewState.h"

class Command;
class StateView;
class DragState;
class TransformBox;
class TransformCommand;

class TransformBox : public ChannelTransform,
					 public ViewState {

 public:

	// TransformBox::Listener
	class Listener {
	 public:
								Listener();
		virtual					~Listener();
	
		virtual	void			TransformBoxDeleted(
									const TransformBox* box) = 0;
	};

	// TransformBox
								TransformBox(StateView* view, BRect box);
	virtual						~TransformBox();

	// ViewState interface
	virtual	void				Init();
	virtual	void				Cleanup();

	virtual	void				Draw(BView* into, BRect updateRect);

	virtual	void				MouseDown(BPoint where, uint32 buttons,
									uint32 clicks);
	virtual	void				MouseMoved(BPoint where, uint32 transit,
									const BMessage* dragMessage);
	virtual	Command*			MouseUp();
			void				MouseOver(BPoint where);
	virtual	bool				DoubleClicked(BPoint where);

	virtual	BRect				Bounds();
//	virtual	BRect				TrackingBounds(BView* withinView);

	virtual	void				ModifiersChanged(uint32 modifiers);

	virtual	bool				HandleKeyDown(const StateView::KeyEvent& event,
									Command** _command);
	virtual	bool				HandleKeyUp(const StateView::KeyEvent& event,
									Command** _command);

	virtual	bool				UpdateCursor();

	// TransformBox
	virtual	void				Update(bool deep = true);

			void				OffsetCenter(BPoint offset);
			BPoint				Center() const;
			void				SetBox(BRect box);
			BRect				Box() const
									{ return fOriginalBox; }

			Command*			FinishTransaction();

			void				NudgeBy(BPoint offset);
			bool				IsNudging() const
									{ return fNudging; }
			Command*			FinishNudging();

	virtual	void				TransformFromCanvas(BPoint& point) const;
	virtual	void				TransformToCanvas(BPoint& point) const;
	virtual	float				ZoomLevel() const;


	virtual	TransformCommand*	MakeCommand(const char* actionName,
											uint32 nameIndex) = 0;

			bool				IsRotating() const
									{ return fCurrentState == fRotateState; }
	virtual	double				ViewSpaceRotation() const;

	// Listener support
			bool				AddListener(TransformBoxListener* listener);
			bool				RemoveListener(TransformBoxListener* listener);

 private:
			DragState*			_DragStateFor(BPoint canvasWhere,
											  float canvasZoom);
			void				_StrokeBWLine(BView* into,
											  BPoint from, BPoint to) const;
			void				_StrokeBWPoint(BView* into,
											   BPoint point,
											   double angle) const;

			BRect				fOriginalBox;

			BPoint				fLeftTop;
			BPoint				fRightTop;
			BPoint				fLeftBottom;
			BPoint				fRightBottom;

			BPoint				fPivot;
			BPoint				fPivotOffset;

			TransformCommand*	fCurrentCommand;
			DragState*			fCurrentState;

			bool				fDragging;
			BPoint				fMousePos;
			uint32				fModifiers;

			bool				fNudging;

			BList				fListeners;

 protected:
			void				_NotifyDeleted() const;

			// "static" state objects
			void				_SetState(DragState* state);

			StateView*			fView;

			DragState*			fDragLTState;
			DragState*			fDragRTState;
			DragState*			fDragLBState;
			DragState*			fDragRBState;

			DragState*			fDragLState;
			DragState*			fDragRState;
			DragState*			fDragTState;
			DragState*			fDragBState;

			DragState*			fRotateState;
			DragState*			fTranslateState;
			DragState*			fOffsetCenterState;
};

#endif // TRANSFORM_BOX_H
