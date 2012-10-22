#include "PlatformScrollArea.h"

#include <QScrollBar>

#include "Scroller.h"


#define AUTO_SCROLL_DELAY		40000 // 40 ms
#define USE_DELAYED_SCROLLING	0
#define USE_NATIVE_SCROLLING	1


class PlatformScrollArea::ScrollerAdapter : public Scroller {
public:
	ScrollerAdapter(PlatformScrollArea* scrollArea)
		:
		fScrollArea(scrollArea)
	{
		SetScrollTarget(fScrollArea);
	}

	virtual void DataRectChanged(BRect oldDataRect, BRect newDataRect)
	{
//		if (_UpdateScrollBarVisibility())
//			_Layout(0);
//		else
			_UpdateScrollBars();
	}

	virtual void ScrollOffsetChanged(BPoint oldOffset, BPoint newOffset)
	{
		fScrollArea->horizontalScrollBar()->setValue(newOffset.x);
		fScrollArea->verticalScrollBar()->setValue(newOffset.y);
	}

	virtual void VisibleSizeChanged(float oldWidth, float oldHeight,
		float newWidth, float newHeight)
	{
//		if (_UpdateScrollBarVisibility())
//			_Layout(0);
//		else
			_UpdateScrollBars();
	}

private:
	void _UpdateScrollBars()
	{
		BRect dataRect = DataRect();
		BRect visibleBounds = VisibleBounds();

		float hMaxValue = max_c(dataRect.left,
			dataRect.right - visibleBounds.Width());
		float vMaxValue = max_c(dataRect.top,
			dataRect.bottom - visibleBounds.Height());

		// update horizontal scroll bar
		if (QScrollBar* hScrollBar = fScrollArea->horizontalScrollBar()) {
			hScrollBar->setRange(dataRect.left, hMaxValue);
			hScrollBar->setPageStep(visibleBounds.IntegerWidth() + 1);
		}

		// update vertical scroll bar
		if (QScrollBar* vScrollBar = fScrollArea->verticalScrollBar()) {
			vScrollBar->setRange(dataRect.top, vMaxValue);
			vScrollBar->setPageStep(visibleBounds.IntegerHeight() + 1);
		}

		// update scroll corner
//		if (fScrollCorner)
//			fScrollCorner->SetActive(hProportion < 1.0f || vProportion < 1.0f);
	}

private:
	PlatformScrollArea*	fScrollArea;
};


PlatformScrollArea::PlatformScrollArea(QWidget* parent)
	:
	QAbstractScrollArea(parent),
	fScrollerAdapter(new ScrollerAdapter(this))
{
}


PlatformScrollArea::~PlatformScrollArea()
{
	delete fScrollerAdapter;
}


void
PlatformScrollArea::resizeEvent(QResizeEvent* event)
{
	SetVisibleSize(viewport()->width(), viewport()->height());
}


void
PlatformScrollArea::scrollContentsBy(int dx, int dy)
{
	SetScrollOffset(BPoint(horizontalScrollBar()->value(),
		verticalScrollBar()->value()));
}
