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
#define USE_NATIVE_SCROLLING	1


CanvasView::CanvasView(QWidget* parent)
	:
	PlatformWidgetHandler<QAbstractScrollArea>("canvas view", parent),
	fDocument(NULL),
	fRenderManager(NULL),
	fZoomLevel(1.0),
	fZoomPolicy(ZOOM_POLICY_ENLARGE_PIXELS),
	fStripesBrush(pattern_to_brush(kStripes, kStripesLow, kStripesHigh))
{
}


void
CanvasView::Init(Document* document, RenderManager* manager)
{
	fDocument = document;
	fRenderManager = manager;

//	StateView::AttachedToWindow();

	BMessenger* bitmapListener = new(std::nothrow) BMessenger(this);
	if (bitmapListener == NULL
		|| !fRenderManager->AddBitmapListener(bitmapListener)) {
		delete bitmapListener;
		// TODO: Bail out, throw exception or something...
	}

//	// init data rect for scrolling and center bitmap in the view
//	BRect dataRect = _LayoutCanvas();
//	SetDataRect(dataRect);
//	BRect bounds(Bounds());
//	BPoint dataRectCenter((dataRect.left + dataRect.right) / 2,
//		(dataRect.top + dataRect.bottom) / 2);
//	BPoint boundsCenter((bounds.left + bounds.right) / 2,
//		(bounds.top + bounds.bottom) / 2);
//	BPoint offset = ScrollOffset();
//	offset.x = roundf(offset.x + dataRectCenter.x - boundsCenter.x);
//	offset.y = roundf(offset.y + dataRectCenter.y - boundsCenter.y);
//	SetScrollOffset(offset);
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
				viewport()->update();
			}
			break;
		}

//		case MSG_ZOOM_SET:
//		{
//			double zoom;
//			if (message->FindDouble("zoom", &zoom) == B_OK)
//				SetZoomLevel(zoom);
//			break;
//		}
//		case MSG_ZOOM_IN:
//			SetZoomLevel(NextZoomInLevel(fZoomLevel), false);
//			break;
//		case MSG_ZOOM_OUT:
//			SetZoomLevel(NextZoomOutLevel(fZoomLevel), false);
//			break;
//		case MSG_ZOOM_ORIGINAL:
//			SetZoomLevel(1.0, false);
//			break;
//		case MSG_ZOOM_TO_FIT:
//		{
//			printf("MSG_ZOOM_TO_FIT\n");
//			break;
//		}

		default:
//			StateView::MessageReceived(message);
			break;
	}
}


void
CanvasView::ConvertFromCanvas(BPoint* point) const
{
	point->x *= fZoomLevel;
	point->y *= fZoomLevel;
#if !USE_NATIVE_SCROLLING
	*point -= ScrollOffset();
#endif
}


void
CanvasView::ConvertToCanvas(BPoint* point) const
{
#if !USE_NATIVE_SCROLLING
	*point += ScrollOffset();
#endif
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

#if !USE_NATIVE_SCROLLING
	r->OffsetBy(-ScrollOffset());
#endif
}


void
CanvasView::ConvertToCanvas(BRect* r) const
{
#if !USE_NATIVE_SCROLLING
	r->OffsetBy(ScrollOffset());
#endif

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
CanvasView::paintEvent(QPaintEvent* event)
{
	BRect canvas(_CanvasRect());

	QPainter painter(viewport());

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
//	BRegion outside(Bounds() & updateRect);
//	outside.Exclude(canvas);
//	FillRegion(&outside, kStripes);
	QRegion outside = event->region().subtracted(canvas.ToQRect());
	if (!outside.isEmpty())
	{
		painter.setClipRegion(outside);
		painter.fillRect(outside.boundingRect(), fStripesBrush);
	}

	//	StateView::Draw(this, updateRect);
}


void
CanvasView::resizeEvent(QResizeEvent* /*event*/)
{
	QSize viewportSize = viewport()->size();
	QSize canvasSize = _CanvasRect().ToQRect().size();

	verticalScrollBar()->setPageStep(viewportSize.height());
	horizontalScrollBar()->setPageStep(viewportSize.width());
	verticalScrollBar()->setRange(0, canvasSize.height() - viewportSize.height());
	horizontalScrollBar()->setRange(0, canvasSize.width() - viewportSize.width());

	_UpdateScrollPosition();
}


void
CanvasView::scrollContentsBy(int dx, int dy)
{
	_UpdateScrollPosition();
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

void
CanvasView::_UpdateScrollPosition()
{
	int hvalue = horizontalScrollBar()->value();
	int vvalue = verticalScrollBar()->value();
	QPoint topLeft = viewport()->rect().topLeft();

//	widget->move(topLeft.x() - hvalue, topLeft.y() - vvalue);
}
