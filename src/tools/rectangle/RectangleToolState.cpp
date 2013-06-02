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

// #pragma mark -

// constructor
RectangleToolState::RectangleToolState(StateView* view, Document* document,
		Selection* selection, CurrentColor* color,
		const BMessenger& configView)
	: DragStateViewState(view)

	, fPlatformDelegate(new PlatformDelegate(this))

	, fCreateRectangleState(new(std::nothrow) CreateRectangleState(this))

	, fDocument(document)
	, fSelection(selection)
	, fCurrentColor(color)

	, fConfigViewMessenger(configView)

	, fInsertionLayer(NULL)
	, fInsertionIndex(-1)

	, fRectangle(NULL)
	
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
			if (style != NULL)
				style->SetFillPaint(Paint(fCurrentColor->Color()));
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
	}

	fRectangle = rectangle;

	if (fRectangle != NULL) {
		fRectangle->AddReference();
		fRectangle->AddListener(this);
	}

	if (rectangle != NULL) {
		if (modifySelection)
			fSelection->Select(Selectable(rectangle), this);

		SetObjectToCanvasTransformation(fRectangle->Transformation());
//		ObjectChanged(rectangle);

		Style* style = fRectangle->Style();
		Paint* fillPaint = style->FillPaint();
		if (fillPaint->Type() == Paint::COLOR) {
			fIgnoreColorNotifiactions = true;
			fCurrentColor->SetColor(fillPaint->Color());
			fIgnoreColorNotifiactions = false;
		}
	} else {
		if (modifySelection)
			fSelection->DeselectAll(this);
//		SetObjectToCanvasTransformation(Transformable());
	}
}

// #pragma mark - private

// _DrawControls
void
RectangleToolState::_DrawControls(PlatformDrawContext& drawContext)
{
}

#endif	// _RECTANGLETOOLSTATE_CPP_
