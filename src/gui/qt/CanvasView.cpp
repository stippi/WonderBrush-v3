#include "CanvasView.h"

#include <Bitmap.h>
#include <Messenger.h>
#include <Region.h>

#include <QPainter>
#include <QPaintEvent>
#include <QRegion>
#include <QScrollBar>

#include "Document.h"
#include "platform_support_ui.h"
#include "RenderManager.h"
#include "ui_defines.h"


#define AUTO_SCROLL_DELAY		40000 // 40 ms
#define USE_DELAYED_SCROLLING	0


CanvasView::CanvasView(QWidget* parent)
	:
	StateView(BRect(), "canvas view", 0, 0),
	fDocument(NULL),
	fRenderManager(NULL),
	fZoomLevel(1.0),
	fZoomPolicy(ZOOM_POLICY_ENLARGE_PIXELS),

	fSpaceHeldDown(false),
	fScrollTracking(false),
	fInScrollTo(false),
	fScrollTrackingStart(0.0, 0.0),
	fScrollOffsetStart(0.0, 0.0),
	fDelayedScrolling(false),

//	fAutoScroller(NULL),

	fStripesBrush(pattern_to_brush(kStripes, kStripesLow, kStripesHigh))
{
	setParent(parent);
}


void
CanvasView::Init(Document* document, RenderManager* manager)
{
	fDocument = document;
	fRenderManager = manager;
}


void
CanvasView::MessageReceived(BMessage* message)
{
	switch (message->what) {
//		case MSG_AUTO_SCROLL:
//			if (fAutoScroller) {
//				BPoint scrollOffset(0.0, 0.0);
//				BRect bounds(Bounds());
//				BPoint mousePos = MouseInfo()->position;
//				mousePos.ConstrainTo(bounds);
//				float inset = min_c(min_c(40.0, bounds.Width() / 10),
//					min_c(40.0, bounds.Height() / 10));
//				bounds.InsetBy(inset, inset);
//				if (!bounds.Contains(mousePos)) {
//					// mouse is close to the border
//					if (mousePos.x <= bounds.left)
//						scrollOffset.x = mousePos.x - bounds.left;
//					else if (mousePos.x >= bounds.right)
//						scrollOffset.x = mousePos.x - bounds.right;
//					if (mousePos.y <= bounds.top)
//						scrollOffset.y = mousePos.y - bounds.top;
//					else if (mousePos.y >= bounds.bottom)
//						scrollOffset.y = mousePos.y - bounds.bottom;

//					scrollOffset.x = roundf(scrollOffset.x * 0.8);
//					scrollOffset.y = roundf(scrollOffset.y * 0.8);
//				}
//				if (scrollOffset != B_ORIGIN) {
//					SetScrollOffset(ScrollOffset() + scrollOffset);
//				}
//			}
//			break;
		case MSG_BITMAP_CLEAN: {
#if USE_DELAYED_SCROLLING
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
				update();
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
	BRect bounds(BRect::FromQRect(rect()));
	BPoint dataRectCenter((dataRect.left + dataRect.right) / 2,
		(dataRect.top + dataRect.bottom) / 2);
	BPoint boundsCenter((bounds.left + bounds.right) / 2,
		(bounds.top + bounds.bottom) / 2);
	BPoint offset = ScrollOffset();
	offset.x = roundf(offset.x + dataRectCenter.x - boundsCenter.x);
	offset.y = roundf(offset.y + dataRectCenter.y - boundsCenter.y);
	SetScrollOffset(offset);
}


void
CanvasView::ConvertFromCanvas(BPoint* point) const
{
	point->x *= fZoomLevel;
	point->y *= fZoomLevel;
	*point -= ScrollOffset();
}


void
CanvasView::ConvertToCanvas(BPoint* point) const
{
	*point += ScrollOffset();
	point->x /= fZoomLevel;
	point->y /= fZoomLevel;
}


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

	r->OffsetBy(-ScrollOffset());
}


void
CanvasView::ConvertToCanvas(BRect* r) const
{
	r->OffsetBy(ScrollOffset());
	r->left /= fZoomLevel;
	r->right /= fZoomLevel;
	r->top /= fZoomLevel;
	r->bottom /= fZoomLevel;
}


float
CanvasView::ZoomLevel() const
{
	return fZoomLevel;
}


void
CanvasView::SetScrollOffset(BPoint newOffset)
{
	if (fInScrollTo)
		return;

	fInScrollTo = true;
#if USE_DELAYED_SCROLLING
	fDelayedScrolling = true;
#endif

	newOffset = ValidScrollOffsetFor(newOffset);
	if (!fScrollTracking) {
#if USE_DELAYED_SCROLLING
		MouseMoved(fMouseInfo.position, fMouseInfo.transit, NULL);
#else
//		BPoint mouseOffset = newOffset - ScrollOffset();
//		MouseMoved(fMouseInfo.position + mouseOffset, fMouseInfo.transit,
//			NULL);
#endif
	}

	Scrollable::SetScrollOffset(newOffset);

	fInScrollTo = false;
}


void
CanvasView::ScrollOffsetChanged(BPoint oldOffset, BPoint newOffset)
{
	BPoint offset = newOffset - oldOffset;

	if (offset == B_ORIGIN) {
		// prevent circular code (MouseMoved might call ScrollBy...)
		return;
	}

#if USE_DELAYED_SCROLLING
	fDelayedScrolling = fRenderManager->ScrollBy(offset);
	if (!fDelayedScrolling)
		Invalidate();
#else
	scroll(offset.x, offset.y);
#endif
	// TODO: Move delayed scrolling stuff into this method:
	fRenderManager->SetCanvasLayout(DataRect(), VisibleRect());
}


void
CanvasView::VisibleSizeChanged(float oldWidth, float oldHeight,
							   float newWidth, float newHeight)
{
	BRect dataRect(_LayoutCanvas());
	SetDataRect(dataRect);

	fRenderManager->SetCanvasLayout(dataRect, VisibleRect());
}


// #pragma mark -


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


void
CanvasView::SetZoomLevel(double zoomLevel, bool mouseIsAnchor)
{
	if (fZoomLevel == zoomLevel)
		return;

	BPoint anchor;
	if (mouseIsAnchor) {
		// zoom into mouse position
//		anchor = MouseInfo()->position;
anchor = BPoint::FromQPoint(QCursor::pos());
	} else {
		// zoom into center of view
		BRect bounds(BRect::FromQRect(rect()));
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

	SetDataRectAndScrollOffset(dataRect, offset);

	_SetRenderManagerZoom();
}


void
CanvasView::SetZoomPolicy(uint32 policy)
{
	if (fZoomPolicy == policy)
		return;

	fZoomPolicy = policy;
	_SetRenderManagerZoom();
}


void
CanvasView::InvalidateCanvas(const BRect& bounds)
{
#if USE_DELAYED_SCROLLING
	if (fDelayedScrolling)
		return;
#endif
	update(bounds.ToQRect());
}


void
CanvasView::paintEvent(QPaintEvent* event)
{
	BRect canvas(_CanvasRect());

	QPainter painter(this);

	// draw document bitmap
	if (fRenderManager->LockDisplay()) {
		const BBitmap* bitmap = fRenderManager->DisplayBitmap();
		if (bitmap->GetQImage() != NULL) {
			painter.drawImage(canvas.ToQRect(), * bitmap->GetQImage(),
				bitmap->Bounds().ToQRect());
		}

		fRenderManager->UnlockDisplay();
	} else {
		painter.fillRect(canvas.ToQRect(), kStripesHigh);
	}

	// outside canvas
	QRegion outside = event->region().subtracted(canvas.ToQRect());
	if (!outside.isEmpty())
	{
		painter.setClipRegion(outside);
		painter.fillRect(outside.boundingRect(), fStripesBrush);
	}

	//	StateView::Draw(this, updateRect);
}


BRect
CanvasView::_CanvasRect() const
{
	if (fDocument == NULL)
		return BRect();

	BRect r(fDocument->Bounds());
	ConvertFromCanvas(&r);
	return r;
}


BRect
CanvasView::_LayoutCanvas()
{
	// size of zoomed bitmap
	BRect r(_CanvasRect());
	r.OffsetTo(0, 0);

	// ask current view state to extend size
//	BRect stateBounds = ViewStateBounds();

	// resize for empty area around bitmap
	// (the size we want, but might still be much smaller than view)
	r.InsetBy(-50, -50);

	// center data rect in bounds
	BRect bounds(BRect::FromQRect(rect()));
	if (bounds.Width() > r.Width())
		r.InsetBy(-ceilf((bounds.Width() - r.Width()) / 2), 0);
	if (bounds.Height() > r.Height())
		r.InsetBy(0, -ceilf((bounds.Height() - r.Height()) / 2));

//	if (stateBounds.IsValid()) {
//		stateBounds.InsetBy(-20, -20);
//		r = r | stateBounds;
//	}

	return r;
}


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
