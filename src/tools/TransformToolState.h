/*
 * Copyright 2009-2010 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef TRANSFORM_TOOL_STATE_H
#define TRANSFORM_TOOL_STATE_H

#include "AbstractLOAdapter.h"
#include "DragStateViewState.h"
#include "Rect.h"
#include "Selection.h"
#include "Shape.h"

class Document;
class Layer;
class Object;

class TransformToolState : public DragStateViewState,
	public Selection::Controller, public Selection::Listener {
private:
	class RectLOAdapater : public RectListener,
		public AbstractLOAdapter {
	public:
								RectLOAdapater(BHandler* handler);
		virtual					~RectLOAdapater();

		virtual	void			AreaChanged(Rect* rect,
									const BRect& oldArea,
									const BRect& newArea);
		virtual	void			Deleted(Rect* rect);
	};

	class ShapeLOAdapater : public ShapeListener,
		public AbstractLOAdapter {
	public:
								ShapeLOAdapater(BHandler* handler);
		virtual					~ShapeLOAdapater();

		virtual	void			AreaChanged(Shape* shape,
									const BRect& oldArea,
									const BRect& newArea);
		virtual	void			Deleted(Shape* shape);
	};


public:
								TransformToolState(StateView* view,
									const BRect& box, Document* document,
									Selection* selection);
	virtual						~TransformToolState();

	// ViewState interface
	virtual	bool				MessageReceived(BMessage* message,
									Command** _command);
	virtual void				MouseDown(BPoint where, uint32 buttons,
									uint32 clicks);
	virtual void				MouseMoved(BPoint where, uint32 transit,
									const BMessage* dragMessage);
	virtual Command*			MouseUp();

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

	// TransformToolState
			void				SetObject(Object* object,
									bool modifySelection = false);
			void				SetTransformable(Transformable* object);
			void				SetBox(const BRect& box);
			void				SetModifiedBox(const BRect& box,
									bool apply = true);
	inline	const BRect&		Box() const
									{ return fOriginalBox; }
	inline	const BRect&		ModifiedBox() const
									{ return fModifiedBox; }

			float				LocalXScale() const;
			float				LocalYScale() const;

private:
			void				_RegisterObject(Transformable* object);
			void				_UnregisterObject(Transformable* object);

private:
			BRect				fOriginalBox;
			BRect				fModifiedBox;

private:
			class PickObjectState;
			class DragBoxState;
			class DragCornerState;
			class DragSideState;

			friend class PickObjectState;

			PickObjectState*	fPickObjectState;

			DragBoxState*		fDragBoxState;

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

			Transformable*		fObject;
			Transformable		fOriginalTransformation;

			RectLOAdapater		fRectLOAdapter;
			ShapeLOAdapater		fShapeLOAdapter;
};

#endif // TRANSFORM_TOOL_STATE_H
