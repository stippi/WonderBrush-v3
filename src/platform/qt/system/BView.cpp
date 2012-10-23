#include "BView.h"

#include <Region.h>


BView::BView(const char* name, uint32 flags)
	:
	PlatformWidgetHandler<QWidget>(name)
{
}


BView::BView(BRect frame, const char* name, uint32 resizeMask, uint32 flags)
	:
	PlatformWidgetHandler<QWidget>(name)
{
}


BView::~BView()
{
}


BWindow*
BView::Window() const
{
// TODO:...
	return NULL;
}


void
BView::Invalidate(BRect invalRect)
{
	update(invalRect.ToQRect());
}


void
BView::Invalidate(const BRegion* invalRegion)
{
// TODO:...
//	update(invalRegion->ToQRegion());
	update(invalRegion->Frame().ToQRect());
}


void
BView::Invalidate()
{
	update();
}


BPoint
BView::ConvertToScreen(BPoint pt) const
{
	return pt + BPoint::FromQPoint(mapToGlobal(QPoint(0, 0)));
}


BPoint
BView::ConvertFromScreen(BPoint pt) const
{
	return pt + BPoint::FromQPoint(mapFromGlobal(QPoint(0, 0)));
}


void
BView::SetViewCursor(const BCursor* cursor, bool sync)
{
	// TODO:...
}


void
BView::SetViewColor(rgb_color c)
{
// TODO:...
}


status_t
BView::SetMouseEventMask(uint32 mask, uint32 options)
{
	// TODO:...
	return B_ERROR;
}


void
BView::SetExplicitMaxSize(BSize size)
{
// TODO:...
}


void
BView::AttachedToWindow()
{
}


void
BView::DetachedFromWindow()
{
}


void
BView::KeyDown(const char* bytes, int32 numBytes)
{
}


void
BView::KeyUp(const char* bytes, int32 numBytes)
{
}
