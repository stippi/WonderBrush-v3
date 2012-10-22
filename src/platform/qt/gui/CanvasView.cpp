#include "CanvasView.h"

#include <Bitmap.h>
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
	QAbstractScrollArea(parent),
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
