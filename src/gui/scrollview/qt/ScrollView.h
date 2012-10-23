#ifndef PLATFORM_QT_PLATFORM_SCROLL_AREA_H
#define PLATFORM_QT_PLATFORM_SCROLL_AREA_H


#include <QAbstractScrollArea>

#include "Scroller.h"


class ScrollView : public QAbstractScrollArea, public Scroller
{
	Q_OBJECT

public:
								ScrollView(QWidget* parent = NULL);
								~ScrollView();

protected:
	virtual	void				resizeEvent(QResizeEvent* event);
	virtual	void				scrollContentsBy(int dx, int dy);

protected:
	virtual	void				DataRectChanged(BRect oldDataRect,
									BRect newDataRect);
	virtual	void				ScrollOffsetChanged(BPoint oldOffset,
									BPoint newOffset);
	virtual	void				VisibleSizeChanged(float oldWidth,
									float oldHeight, float newWidth,
									float newHeight);
	virtual	void				ScrollTargetChanged(Scrollable* oldTarget,
									Scrollable* newTarget);

private:
			void				_UpdateScrollBars();

private:
			QWidget*			fChild;
};


#endif // PLATFORM_QT_PLATFORM_SCROLL_AREA_H
