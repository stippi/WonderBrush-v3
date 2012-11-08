#include "CanvasView.h"
#include "CanvasViewPlatformDelegate.h"

#include <stdio.h>

#include <Bitmap.h>
#include <Cursor.h>
#include <LayoutUtils.h>
#include <Message.h>
#include <MessageRunner.h>
#include <Messenger.h>
#include <Region.h>
#include <Window.h>

#include "cursors.h"
#include "ui_defines.h"
#include "support.h"

#include "Document.h"
#include "RenderManager.h"


enum {
	MSG_AUTO_SCROLL	= 'ascr'
};

// constructor
CanvasView::CanvasView(BRect frame, Document* document, RenderManager* manager)
	: StateView(frame, "canvas view", B_FOLLOW_NONE,
		B_WILL_DRAW | B_FRAME_EVENTS)
	, fPlatformDelegate(new PlatformDelegate(this))
	, fDocument(document)
	, fRenderManager(manager)

	, fZoomLevel(1.0)
	, fZoomPolicy(ZOOM_POLICY_ENLARGE_PIXELS)

	, fSpaceHeldDown(false)
	, fScrollTracking(false)
	, fInScrollTo(false)
	, fScrollTrackingStart(0.0, 0.0)
	, fDelayedScrolling(false)

	, fAutoScroller(NULL)
{
	SetLocker(fDocument);
}


// constructor
CanvasView::CanvasView(Document* document, RenderManager* manager)
	: StateView("canvas view", B_WILL_DRAW | B_FRAME_EVENTS)
	, fPlatformDelegate(new PlatformDelegate(this))
	, fDocument(document)
	, fRenderManager(manager)

	, fZoomLevel(1.0)
	, fZoomPolicy(ZOOM_POLICY_ENLARGE_PIXELS)

	, fSpaceHeldDown(false)
	, fScrollTracking(false)
	, fInScrollTo(false)
	, fScrollTrackingStart(0.0, 0.0)
	, fDelayedScrolling(false)

	, fAutoScroller(NULL)
{
	SetLocker(fDocument);
}


// destructor
CanvasView::~CanvasView()
{
	delete fAutoScroller;
	delete fPlatformDelegate;
}

// MessageReceived
void
CanvasView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_AUTO_SCROLL:
			if (fAutoScroller) {
				BPoint scrollOffset(0.0, 0.0);
				BRect bounds(Bounds());
				BPoint mousePos = MouseInfo()->position;
				mousePos.ConstrainTo(bounds);
				float inset = min_c(min_c(40.0, bounds.Width() / 10),
					min_c(40.0, bounds.Height() / 10));
				bounds.InsetBy(inset, inset);
				if (!bounds.Contains(mousePos)) {
					// mouse is close to the border
					if (mousePos.x <= bounds.left)
						scrollOffset.x = mousePos.x - bounds.left;
					else if (mousePos.x >= bounds.right)
						scrollOffset.x = mousePos.x - bounds.right;
					if (mousePos.y <= bounds.top)
						scrollOffset.y = mousePos.y - bounds.top;
					else if (mousePos.y >= bounds.bottom)
						scrollOffset.y = mousePos.y - bounds.bottom;

					scrollOffset.x = roundf(scrollOffset.x * 0.8);
					scrollOffset.y = roundf(scrollOffset.y * 0.8);
				}
				if (scrollOffset != B_ORIGIN) {
					SetScrollOffset(ScrollOffset() + scrollOffset);
				}
			}
			break;
		case MSG_BITMAP_CLEAN: {
#if CANVAS_VIEW_USE_DELAYED_SCROLLING
			bool scrollingDelayed;
			if (message->FindBool("scrolling delayed",
				&scrollingDelayed) == B_OK) {
				fDelayedScrolling = false;
				// just invalidate everything, it will simulate scrolling,
				// but only after rendering is done
				Invalidate();
				break;
			}
#endif
			BRect area;
			if (message->FindRect("area", &area) == B_OK) {
				ConvertFromCanvas(&area);
				area.left = floorf(area.left);
				area.top = floorf(area.top);
				area.right = ceilf(area.right);
				area.bottom = ceilf(area.bottom);
				Invalidate(area);
			}
			break;
		}

		case MSG_ZOOM_SET:
		{
			double zoom;
			if (message->FindDouble("zoom", &zoom) == B_OK)
				SetZoomLevel(zoom);
			break;
		}
		case MSG_ZOOM_IN:
			SetZoomLevel(NextZoomInLevel(fZoomLevel), false);
			break;
		case MSG_ZOOM_OUT:
			SetZoomLevel(NextZoomOutLevel(fZoomLevel), false);
			break;
		case MSG_ZOOM_ORIGINAL:
			SetZoomLevel(1.0, false);
			break;
		case MSG_ZOOM_TO_FIT:
		{
			printf("MSG_ZOOM_TO_FIT\n");
			break;
		}

		default:
			StateView::MessageReceived(message);
			break;
	}
}

// AttachedToWindow
void
CanvasView::AttachedToWindow()
{
	StateView::AttachedToWindow();

	BMessenger* bitmapListener = new(std::nothrow) BMessenger(this);
	if (bitmapListener == NULL
		|| !fRenderManager->AddBitmapListener(bitmapListener)) {
		delete bitmapListener;
		// TODO: Bail out, throw exception or something...
	}

	// init data rect for scrolling and center bitmap in the view
	BRect dataRect = _LayoutCanvas();
	SetDataRect(dataRect);
	BRect bounds(Bounds());
	BPoint dataRectCenter((dataRect.left + dataRect.right) / 2,
		(dataRect.top + dataRect.bottom) / 2);
	BPoint boundsCenter((bounds.left + bounds.right) / 2,
		(bounds.top + bounds.bottom) / 2);
	BPoint offset = ScrollOffset();
	offset.x = roundf(offset.x + dataRectCenter.x - boundsCenter.x);
	offset.y = roundf(offset.y + dataRectCenter.y - boundsCenter.y);
	SetScrollOffset(offset);
}

// FrameResized
void
CanvasView::FrameResized(float width, float height)
{
	StateView::FrameResized(width, height);
}

void
CanvasView::GetPreferredSize(float* _width, float* _height)
{
	if (_width != NULL)
		*_width = 100;
	if (_height != NULL)
		*_height = 100;
}


BSize
CanvasView::MaxSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMaxSize(),
		BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));
}


void
CanvasView::PlatformDraw(PlatformDrawContext& drawContext)
{
	BRect canvas(_CanvasRect());

	// draw document bitmap
	if (fRenderManager->LockDisplay()) {
		const BBitmap* bitmap = fRenderManager->DisplayBitmap();
		fPlatformDelegate->DrawCanvas(drawContext, bitmap, canvas);
		fRenderManager->UnlockDisplay();
	} else
		fPlatformDelegate->DrawCanvas(drawContext, NULL, canvas);

	// outside canvas
	fPlatformDelegate->DrawStripes(drawContext, canvas);

	StateView::Draw(drawContext);
}

// #pragma mark -

// MouseDown
void
CanvasView::MouseDown(BPoint where)
{
	if (!IsFocus())
		MakeFocus(true);

	uint32 buttons;
	if (Window()->CurrentMessage()->FindInt32("buttons",
		(int32*)&buttons) != B_OK) {
		buttons = 0;
	}

	// handle clicks of the third mouse button ourself (panning),
	// otherwise have StateView handle it (normal clicks)
	if (fSpaceHeldDown || buttons & B_TERTIARY_MOUSE_BUTTON) {
		// switch into scrolling mode and update cursor
		fScrollTracking = true;
		where.x = roundf(where.x);
		where.y = roundf(where.y);
		fScrollTrackingStart = where;
		ConvertToCanvas(&fScrollTrackingStart);
		_UpdateToolCursor();
		SetMouseEventMask(B_POINTER_EVENTS,
						  B_LOCK_WINDOW_FOCUS | B_SUSPEND_VIEW_FOCUS);
	} else {
		SetAutoScrolling(true);
		StateView::MouseDown(where);
	}
}

// MouseUp
void
CanvasView::MouseUp(BPoint where)
{
	if (fScrollTracking) {
		// stop scroll tracking and update cursor
		fScrollTracking = false;
		_UpdateToolCursor();
		// update StateView mouse position
		uint32 transit = Bounds().Contains(where) ?
			B_INSIDE_VIEW : B_OUTSIDE_VIEW;
		StateView::MouseMoved(where, transit, NULL);
	} else {
		StateView::MouseUp(where);
	}
	SetAutoScrolling(false);
}

// MouseMoved
void
CanvasView::MouseMoved(BPoint where, uint32 transit, const BMessage* dragMessage)
{
	if (fScrollTracking) {
		uint32 buttons;
		GetMouse(&where, &buttons, false);
		if (!buttons) {
			MouseUp(where);
			return;
		}
		where.x = roundf(where.x);
		where.y = roundf(where.y);
		ConvertToCanvas(&where);
		SetScrollOffset(ScrollOffset() + fScrollTrackingStart - where);
	} else {
		// normal mouse movement handled by StateView
//		if (!fSpaceHeldDown)
			StateView::MouseMoved(where, transit, dragMessage);
	}
	_UpdateToolCursor();
}

// #pragma mark -

// MouseWheelChanged
bool
CanvasView::MouseWheelChanged(BPoint where, float x, float y)
{
	if (!Bounds().Contains(where))
		return false;

	if (y > 0.0) {
		SetZoomLevel(NextZoomOutLevel(fZoomLevel), true);
		return true;
	} else if (y < 0.0) {
		SetZoomLevel(NextZoomInLevel(fZoomLevel), true);
		return true;
	}
	return false;
}

// ViewStateBoundsChanged
void
CanvasView::ViewStateBoundsChanged()
{
	if (!Window())
		return;
//	if (fScrollTracking)
//		return;

//	fScrollTracking = true;
	SetDataRect(_LayoutCanvas());
//	fScrollTracking = false;
}

// #pragma mark -

// ConvertFromCanvas
void
CanvasView::ConvertFromCanvas(BPoint* point) const
{
	point->x *= fZoomLevel;
	point->y *= fZoomLevel;
#if !CANVAS_VIEW_USE_NATIVE_SCROLLING
	*point -= ScrollOffset();
#endif
}

// ConvertToCanvas
void
CanvasView::ConvertToCanvas(BPoint* point) const
{
#if !CANVAS_VIEW_USE_NATIVE_SCROLLING
	*point += ScrollOffset();
#endif
	point->x /= fZoomLevel;
	point->y /= fZoomLevel;
}

// ConvertFromCanvas
void
CanvasView::ConvertFromCanvas(BRect* r) const
{
	r->left *= fZoomLevel;
	r->top *= fZoomLevel;
	r->right++;
	r->bottom++;
	r->right *= fZoomLevel;
	r->bottom *= fZoomLevel;
	r->right--;
	r->bottom--;

#if !CANVAS_VIEW_USE_NATIVE_SCROLLING
	r->OffsetBy(-ScrollOffset());
#endif
}

// ConvertToCanvas
void
CanvasView::ConvertToCanvas(BRect* r) const
{
#if !CANVAS_VIEW_USE_NATIVE_SCROLLING
	r->OffsetBy(ScrollOffset());
#endif

	r->left /= fZoomLevel;
	r->right /= fZoomLevel;
	r->top /= fZoomLevel;
	r->bottom /= fZoomLevel;
}

// ZoomLevel
float
CanvasView::ZoomLevel() const
{
	return fZoomLevel;
}

// #pragma mark -

// SetScrollOffset
void
CanvasView::SetScrollOffset(BPoint newOffset)
{
	if (fInScrollTo)
		return;

	fInScrollTo = true;
#if CANVAS_VIEW_USE_DELAYED_SCROLLING
	fDelayedScrolling = true;
#endif

	newOffset = ValidScrollOffsetFor(newOffset);
	if (!fScrollTracking) {
#if CANVAS_VIEW_USE_DELAYED_SCROLLING
		MouseMoved(fMouseInfo.position, fMouseInfo.transit, NULL);
#else
		BPoint mouseOffset = newOffset - ScrollOffset();
		MouseMoved(fMouseInfo.position + mouseOffset, fMouseInfo.transit,
			NULL);
#endif
	}

	Scrollable::SetScrollOffset(newOffset);

	fInScrollTo = false;
}

// ScrollOffsetChanged
void
CanvasView::ScrollOffsetChanged(BPoint oldOffset, BPoint newOffset)
{
	BPoint offset = newOffset - oldOffset;

	if (offset == B_ORIGIN) {
		// prevent circular code (MouseMoved might call ScrollBy...)
		return;
	}

#if CANVAS_VIEW_USE_DELAYED_SCROLLING
	fDelayedScrolling = fRenderManager->ScrollBy(offset);
	if (!fDelayedScrolling)
		Invalidate();
#else
	fPlatformDelegate->ScrollBy(offset);
#endif
	// TODO: Move delayed scrolling stuff into this method:
	fRenderManager->SetCanvasLayout(DataRect(), VisibleRect());
}

// VisibleSizeChanged
void
CanvasView::VisibleSizeChanged(float oldWidth, float oldHeight,
							   float newWidth, float newHeight)
{
	BRect dataRect(_LayoutCanvas());
	SetDataRect(dataRect);

	fRenderManager->SetCanvasLayout(dataRect, VisibleRect());
}

// #pragma mark -

// NextZoomInLevel
double
CanvasView::NextZoomInLevel(double zoom) const
{
	if (zoom < 0.25)
		return 0.25;
	if (zoom < 0.33)
		return 0.33;
	if (zoom < 0.5)
		return 0.5;
	if (zoom < 0.66)
		return 0.66;
	if (zoom < 1)
		return 1;
	if (zoom < 1.5)
		return 1.5;
	if (zoom < 2)
		return 2;
	if (zoom < 3)
		return 3;
	if (zoom < 4)
		return 4;
	if (zoom < 6)
		return 6;
	if (zoom < 8)
		return 8;
	if (zoom < 16)
		return 16;
	if (zoom < 32)
		return 32;
	return 64;
}

// NextZoomOutLevel
double
CanvasView::NextZoomOutLevel(double zoom) const
{
	if (zoom > 32)
		return 32;
	if (zoom > 16)
		return 16;
	if (zoom > 8)
		return 8;
	if (zoom > 6)
		return 6;
	if (zoom > 4)
		return 4;
	if (zoom > 3)
		return 3;
	if (zoom > 2)
		return 2;
	if (zoom > 1.5)
		return 1.5;
	if (zoom > 1.0)
		return 1.0;
	if (zoom > 0.66)
		return 0.66;
	if (zoom > 0.5)
		return 0.5;
	if (zoom > 0.33)
		return 0.33;
	return 0.25;
}

// SetZoomLevel
void
CanvasView::SetZoomLevel(double zoomLevel, bool mouseIsAnchor)
{
	if (fZoomLevel == zoomLevel)
		return;

	BPoint anchor;
	if (mouseIsAnchor) {
		// zoom into mouse position
		anchor = MouseInfo()->position;
	} else {
		// zoom into center of view
		BRect bounds(Bounds());
		anchor.x = (bounds.left + bounds.right + 1) / 2.0;
		anchor.y = (bounds.top + bounds.bottom + 1) / 2.0;
	}

	BPoint canvasAnchor = anchor;
	ConvertToCanvas(&canvasAnchor);

	fZoomLevel = zoomLevel;
	BRect dataRect = _LayoutCanvas();

	ConvertFromCanvas(&canvasAnchor);

	BPoint offset = ScrollOffset();
	offset.x = roundf(offset.x + canvasAnchor.x - anchor.x);
	offset.y = roundf(offset.y + canvasAnchor.y - anchor.y);

#if CANVAS_VIEW_USE_NATIVE_SCROLLING
	Invalidate();
		// Cause the (Haiku) app_server to skip visual scrolling
#endif

	SetDataRectAndScrollOffset(dataRect, offset);

	_SetRenderManagerZoom();
}

// SetZoomPolicy
void
CanvasView::SetZoomPolicy(uint32 policy)
{
	if (fZoomPolicy == policy)
		return;

	fZoomPolicy = policy;
	_SetRenderManagerZoom();
}

// SetAutoScrolling
void
CanvasView::SetAutoScrolling(bool scroll)
{
	if (scroll) {
		if (!fAutoScroller) {
			BMessenger messenger(this, Window());
			BMessage message(MSG_AUTO_SCROLL);
			// this trick avoids the MouseMoved() hook
			// to think that the mouse is not pressed
			// anymore when we call it ourselfs from the
			// autoscrolling code
			message.AddInt32("buttons", 1);
			fAutoScroller = new BMessageRunner(messenger,
											   &message,
											   CANVAS_VIEW_AUTO_SCROLL_DELAY);
		}
	} else {
		delete fAutoScroller;
		fAutoScroller = NULL;
	}
}

// InvalidateCanvas
void
CanvasView::InvalidateCanvas(const BRect& bounds)
{
#if CANVAS_VIEW_USE_DELAYED_SCROLLING
	if (fDelayedScrolling)
		return;
#endif
	Invalidate(bounds);
}

// #pragma mark -

// _HandleKeyDown
bool
CanvasView::_HandleKeyDown(const StateView::KeyEvent& event,
	BHandler* originalHandler)
{
	switch (event.key) {
		case 'z':
		case 'y':
//			if (modifiers & B_SHIFT_KEY)
//				CommandStack()->Redo();
//			else
//				CommandStack()->Undo();
			break;

		case '+':
			SetZoomLevel(NextZoomInLevel(fZoomLevel));
			break;
		case '-':
			SetZoomLevel(NextZoomOutLevel(fZoomLevel));
			break;

		case B_SPACE:
			fSpaceHeldDown = true;
			_UpdateToolCursor();
			break;

		default:
			return StateView::_HandleKeyDown(event,
				originalHandler);
	}

	return true;
}

// _HandleKeyUp
bool
CanvasView::_HandleKeyUp(const StateView::KeyEvent& event,
	BHandler* originalHandler)
{
	switch (event.key) {
		case B_SPACE:
			fSpaceHeldDown = false;
			_UpdateToolCursor();
			break;

		default:
			return StateView::_HandleKeyUp(event,
				originalHandler);
	}

	return true;
}

// #pragma mark -

// _CanvasRect()
BRect
CanvasView::_CanvasRect() const
{
	BRect r(fDocument->Bounds());
	ConvertFromCanvas(&r);
	return r;
}

// _LayoutCanvas
BRect
CanvasView::_LayoutCanvas()
{
	// size of zoomed bitmap
	BRect r(_CanvasRect());
	r.OffsetTo(B_ORIGIN);

	// ask current view state to extend size
	BRect stateBounds = ViewStateBounds();

	// resize for empty area around bitmap
	// (the size we want, but might still be much smaller than view)
	r.InsetBy(-50, -50);

	// center data rect in bounds
	BRect bounds(Bounds());
	if (bounds.Width() > r.Width())
		r.InsetBy(-ceilf((bounds.Width() - r.Width()) / 2), 0);
	if (bounds.Height() > r.Height())
		r.InsetBy(0, -ceilf((bounds.Height() - r.Height()) / 2));

	if (stateBounds.IsValid()) {
		stateBounds.InsetBy(-20, -20);
		r = r | stateBounds;
	}

	return r;
}

// _SetRenderManagerZoom
void
CanvasView::_SetRenderManagerZoom()
{
	if (fZoomLevel <= 1.0)	
		fRenderManager->SetZoomLevel(fZoomLevel);
	else {
		// upscaling depends on zoom policy
		if (fZoomPolicy == ZOOM_POLICY_ENLARGE_PIXELS)
			fRenderManager->SetZoomLevel(1.0);
		else
			fRenderManager->SetZoomLevel(fZoomLevel);
	}
}

// #pragma mark -

// _UpdateToolCursor
void
CanvasView::_UpdateToolCursor()
{
	if (fScrollTracking || fSpaceHeldDown) {
		// indicate scrolling mode
#ifdef __HAIKU__
		BCursorID cursorID = fScrollTracking ? B_CURSOR_ID_GRABBING
			: B_CURSOR_ID_GRAB;
		BCursor cursor(cursorID);
#else
		const uchar* cursorData = fScrollTracking ? kGrabCursor : kHandCursor;
		BCursor cursor(cursorData);
#endif
		SetViewCursor(&cursor, true);
	} else {
		// pass on to current state of StateView
		UpdateStateCursor();
	}
}
