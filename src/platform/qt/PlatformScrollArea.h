#ifndef PLATFORM_QT_PLATFORM_SCROLL_AREA_H
#define PLATFORM_QT_PLATFORM_SCROLL_AREA_H


#include <QAbstractScrollArea>

#include "Scrollable.h"


class PlatformScrollArea : public QAbstractScrollArea, public Scrollable
{
	Q_OBJECT

public:
								PlatformScrollArea(QWidget* parent = NULL);
								~PlatformScrollArea();

protected:
	virtual	void				resizeEvent(QResizeEvent* event);
	virtual	void				scrollContentsBy(int dx, int dy);

private:
			class ScrollerAdapter;

private:
			ScrollerAdapter*	fScrollerAdapter;
};


#endif // PLATFORM_QT_PLATFORM_SCROLL_AREA_H
