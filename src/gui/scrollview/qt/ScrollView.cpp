#include "ScrollView.h"

#include <QScrollBar>

#include "Scrollable.h"


#define AUTO_SCROLL_DELAY		40000 // 40 ms
#define USE_DELAYED_SCROLLING	0
#define USE_NATIVE_SCROLLING	1


ScrollView::ScrollView(QWidget* parent)
	:
	QAbstractScrollArea(parent)
{
}


ScrollView::~ScrollView()
{
}


void
ScrollView::resizeEvent(QResizeEvent* event)
{
	QAbstractScrollArea::resizeEvent(event);

	fChild->move(0, 0);
	fChild->resize(viewport()->size());

	SetVisibleSize(viewport()->width(), viewport()->height());
}


void
ScrollView::scrollContentsBy(int dx, int dy)
{
	SetScrollOffset(BPoint(horizontalScrollBar()->value(),
		verticalScrollBar()->value()));
}


void
ScrollView::DataRectChanged(BRect oldDataRect, BRect newDataRect)
{
//	if (_UpdateScrollBarVisibility())
//		_Layout(0);
//	else
		_UpdateScrollBars();
}


void
ScrollView::ScrollOffsetChanged(BPoint oldOffset, BPoint newOffset)
{
	horizontalScrollBar()->setValue(newOffset.x);
	verticalScrollBar()->setValue(newOffset.y);
}


void
ScrollView::VisibleSizeChanged(float oldWidth, float oldHeight,
	float newWidth, float newHeight)
{
//	if (_UpdateScrollBarVisibility())
//		_Layout(0);
//	else
		_UpdateScrollBars();
}


void
ScrollView::ScrollTargetChanged(Scrollable* oldTarget, Scrollable* newTarget)
{
	if (QWidget* oldChild = dynamic_cast<QWidget*>(oldTarget))
		oldChild->setParent(NULL);

	fChild = dynamic_cast<QWidget*>(newTarget);

	if (fChild != NULL) {
		fChild->setParent(viewport());
		fChild->move(0, 0);
		fChild->resize(viewport()->size());
	}
}


void
ScrollView::_UpdateScrollBars()
{
	BRect dataRect = DataRect();
	BRect visibleBounds = VisibleBounds();

	float hMaxValue = max_c(dataRect.left,
		dataRect.right - visibleBounds.Width());
	float vMaxValue = max_c(dataRect.top,
		dataRect.bottom - visibleBounds.Height());

	// update horizontal scroll bar
	if (QScrollBar* hScrollBar = horizontalScrollBar()) {
		hScrollBar->setRange(dataRect.left, hMaxValue);
		hScrollBar->setPageStep(visibleBounds.IntegerWidth() + 1);
	}

	// update vertical scroll bar
	if (QScrollBar* vScrollBar = verticalScrollBar()) {
		vScrollBar->setRange(dataRect.top, vMaxValue);
		vScrollBar->setPageStep(visibleBounds.IntegerHeight() + 1);
	}

	// update scroll corner
//	if (fScrollCorner)
//		fScrollCorner->SetActive(hProportion < 1.0f || vProportion < 1.0f);
}
