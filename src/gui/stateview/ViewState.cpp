#include "ViewState.h"

#include "StateView.h"

// constructor
ViewState::ViewState(StateView* view)
	:
	fView(view),
	fMouseInfo(view->MouseInfo()),
	fBounds(0, 0, -1, -1),
	fIsActive(false)
{
}

// constructor
ViewState::ViewState(const ViewState& other)
	:
	fView(other.fView),
	fMouseInfo(other.fMouseInfo),
	fBounds(other.fBounds),
	fIsActive(false)
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
	fIsActive = true;
}

// Cleanup
void
ViewState::Cleanup()
{
	fIsActive = false;
	UpdateBounds();
}

// IsActive
bool
ViewState::IsActive() const
{
	return fIsActive;
}

// #pragma mark -

// Draw
void
ViewState::Draw(PlatformDrawContext& drawContext)
{
}

// MessageReceived
bool
ViewState::MessageReceived(BMessage* message, UndoableEdit** _edit)
{
	return false;
}

// #pragma mark -

// MouseDown
void
ViewState::MouseDown(const MouseInfo& info)
{
}

// MouseMoved
void
ViewState::MouseMoved(const MouseInfo& info)
{
}

// MouseUp
UndoableEdit*
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
						 UndoableEdit** _edit)
{
	return false;
}

// HandleKeyUp
bool
ViewState::HandleKeyUp(const StateView::KeyEvent& event,
					   UndoableEdit** _edit)
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
	fView->InvalidateCanvas(dirty);
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
ViewState::Invalidate() const
{
	Invalidate(fBounds);
}

// Invalidate
void
ViewState::Invalidate(BRect canvasBounds) const
{
	fView->ConvertFromCanvas(&canvasBounds);
	canvasBounds.left = floorf(canvasBounds.left);
	canvasBounds.top = floorf(canvasBounds.top);
	canvasBounds.right = ceilf(canvasBounds.right);
	canvasBounds.bottom = ceilf(canvasBounds.bottom);
	fView->Invalidate(canvasBounds);
}

