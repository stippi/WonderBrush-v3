#include "ViewState.h"

#include "StateView.h"

mouse_info::mouse_info()
	:
	buttons(0),
	position(B_ORIGIN),
	transit(B_OUTSIDE_VIEW),
	modifiers(::modifiers())
{
}

// constructor
ViewState::ViewState(StateView* view)
	:
	fView(view),
	fMouseInfo(view->MouseInfo()),
	fBounds(0, 0, -1, -1)
{
}

// constructor
ViewState::ViewState(const ViewState& other)
	:
	fView(other.fView),
	fMouseInfo(other.fMouseInfo),
	fBounds(other.fBounds)
{
}

// destructor
ViewState::~ViewState()
{
}

// #pragma mark -

// Init
void
ViewState::Init()
{
	UpdateBounds();
}

// Cleanup
void
ViewState::Cleanup()
{
	UpdateBounds();
}

// #pragma mark -

// Draw
void
ViewState::Draw(BView* into, BRect updateRect)
{
}

// MessageReceived
bool
ViewState::MessageReceived(BMessage* message, Command** _command)
{
	return false;
}

// #pragma mark -

// MouseDown
void
ViewState::MouseDown(BPoint where, uint32 buttons, uint32 clicks)
{
}

// MouseMoved
void
ViewState::MouseMoved(BPoint where, uint32 transit, const BMessage* dragMessage)
{
}

// MouseUp
Command*
ViewState::MouseUp()
{
	return NULL;
}

// #pragma mark -

// ModifiersChanged
void
ViewState::ModifiersChanged(uint32 modifiers)
{
}

// HandleKeyDown
bool
ViewState::HandleKeyDown(const StateView::KeyEvent& event,
						 Command** _command)
{
	return false;
}

// HandleKeyUp
bool
ViewState::HandleKeyUp(const StateView::KeyEvent& event,
					   Command** _command)
{
	return false;
}

// UpdateCursor
bool
ViewState::UpdateCursor()
{
	return false;
}

// Bounds
BRect
ViewState::Bounds() const
{
	return BRect(0, 0, -1, -1);
}

// UpdateBounds
void
ViewState::UpdateBounds()
{
	BRect dirty = fBounds;
	fBounds = Bounds();
	if (dirty.IsValid())
		dirty = dirty | fBounds;
	else
		dirty = fBounds;
	fView->Invalidate(dirty);
}

// TransformCanvasToView
void
ViewState::TransformCanvasToView(BPoint* point) const
{
	fView->ConvertFromCanvas(point);
}

// TransformViewToCanvas
void
ViewState::TransformViewToCanvas(BPoint* point) const
{
	fView->ConvertToCanvas(point);
}

// TransformCanvasToView
void
ViewState::TransformCanvasToView(BRect* rect) const
{
	fView->ConvertFromCanvas(rect);
}

// TransformViewToCanvas
void
ViewState::TransformViewToCanvas(BRect* rect) const
{
	fView->ConvertToCanvas(rect);
}

// ZoomLevel
float
ViewState::ZoomLevel() const
{
	return fView->ZoomLevel();
}

// Invalidate
void
ViewState::Invalidate(BRect canvasBounds)
{
	fView->ConvertFromCanvas(&canvasBounds);
	canvasBounds.left = floorf(canvasBounds.left);
	canvasBounds.top = floorf(canvasBounds.top);
	canvasBounds.right = ceilf(canvasBounds.right);
	canvasBounds.bottom = ceilf(canvasBounds.bottom);
	fView->Invalidate(canvasBounds);
}

