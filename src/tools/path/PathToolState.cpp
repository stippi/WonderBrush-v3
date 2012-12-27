#ifndef _PATHTOOLSTATE_CPP_
#define _PATHTOOLSTATE_CPP_

/*
 * Copyright 2012 Stephan Aßmus <superstippi@gmx.de>
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
#include "Document.h"
#include "Layer.h"
#include "ObjectAddedEdit.h"
#include "support.h"
#include "Shape.h"
#include "TransformObjectEdit.h"
#include "ui_defines.h"

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
		// Setup tool and switch to drag left/top state
		if (fParent->CreateShape(origin)) {
//			fParent->SetDragState(fParent->fDragLeftTopState);
//			fParent->fDragLeftTopState->SetOrigin(origin);
		}
	}

	virtual void DragTo(BPoint current, uint32 modifiers)
	{
	}

	virtual BCursor ViewCursor(BPoint current) const
	{
		return BCursor(B_CURSOR_ID_SYSTEM_DEFAULT);
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
	, fCreateShapeState(new(std::nothrow) CreateShapeState(this))

	, fDocument(document)
	, fSelection(selection)
	, fCurrentColor(color)

	, fConfigViewMessenger(configView)

	, fInsertionLayer(NULL)
	, fInsertionIndex(-1)

	, fShape(NULL)

	, fStyle(new(std::nothrow) Style(), true)

	, fIgnoreColorColorNotifiactions(false)
{
	// TODO: Find a way to change this later...
	SetInsertionInfo(fDocument->RootLayer(),
		fDocument->RootLayer()->CountObjects());

	fCurrentColor->AddListener(this);

	fStyle.Get()->SetFillPaint(Paint(fCurrentColor->Color()));
}

// destructor
PathToolState::~PathToolState()
{
	SetShape(NULL);
	fCurrentColor->RemoveListener(this);
	fSelection->RemoveListener(this);

	delete fPickShapeState;
	delete fCreateShapeState;

	SetInsertionInfo(NULL, -1);

	delete fPlatformDelegate;
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
		default:
			handled = DragStateViewState::MessageReceived(message, _edit);
	}

	return handled;
}

// #pragma mark -

// ModifiersChanged
void
PathToolState::ModifiersChanged(uint32 modifiers)
{
	DragStateViewState::ModifiersChanged(modifiers);
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
	if (fShape == NULL)
		return;

	_DrawControls(drawContext);
}

// Bounds
BRect
PathToolState::Bounds() const
{
	if (fShape == NULL)
		return BRect(0, 0, -1, -1);

	BRect bounds = fShape->Bounds();
	bounds.InsetBy(-15, -15);
	TransformObjectToView(&bounds);
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
	double scaleX;
	double scaleY;
	if (fShape != NULL
		&& fShape->GetAffineParameters(NULL, NULL, NULL,
			&scaleX, &scaleY, NULL, NULL)) {
//		float inset = 7.0 / zoomLevel;
	}

	// If there is still no state, switch to the PickObjectsState
	// and try to find an object. If nothing is picked, unset on mouse down.
	Object* pickedObject = NULL;
	fDocument->RootLayer()->HitTest(canvasWhere, NULL, &pickedObject, true);

	Shape* pickedShape = dynamic_cast<Shape*>(pickedObject);
	if (pickedShape != NULL) {
		fPickShapeState->SetShape(pickedShape);
		return fPickShapeState;
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
	if (fShape != NULL && object == fShape) {
		SetObjectToCanvasTransformation(fShape->Transformation());
		UpdateBounds();
		UpdateDragState();
	}
	if (object == fCurrentColor && !fIgnoreColorColorNotifiactions) {
	}
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

	Shape* shape = new(std::nothrow) Shape(BRect(0, 0, 50, 50), kBlack);
	if (shape == NULL) {
		fprintf(stderr, "PathToolState::CreateShape(): Failed to allocate "
			"Shape. Out of memory\n");
		return false;
	}

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

// SetShape
void
PathToolState::SetShape(Shape* shape, bool modifySelection)
{
	if (fShape == shape)
		return;

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
		SetObjectToCanvasTransformation(shape->Transformation());
	} else {
		if (modifySelection)
			fSelection->DeselectAll(this);
		SetObjectToCanvasTransformation(Transformable());
	}
}

// #pragma mark - private

// _DrawControls
void
PathToolState::_DrawControls(PlatformDrawContext& drawContext)
{
	double scaleX;
	double scaleY;
	if (!EffectiveTransformation().GetAffineParameters(NULL, NULL, NULL,
		&scaleX, &scaleY, NULL, NULL)) {
		return;
	}

	scaleX *= fView->ZoomLevel();
	scaleY *= fView->ZoomLevel();

	BPoint origin(0.0f, -10.0f / scaleY);
	TransformObjectToView(&origin, true);

	BPoint widthOffset(0.0f, -10.0f / scaleY);
	TransformObjectToView(&widthOffset, true);

	fPlatformDelegate->DrawControls(drawContext, origin, widthOffset);
}

#endif	// _PATHTOOLSTATE_CPP_
