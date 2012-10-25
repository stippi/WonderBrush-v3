#include "BView.h"

#include <Region.h>


BView::BView(BMessage* archive)
	:
	PlatformWidgetHandler<QWidget>(archive)
{
}


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


BRect
BView::Bounds() const
{
	return BRect::FromQRect(rect());
}


uint32
BView::Flags() const
{
// TODO:...
	return 0;
}


void
BView::SetFlags(uint32 flags)
{
// TODO:...
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


void
BView::InvalidateLayout(bool descendants)
{
// TODO: Handle descendants!
	updateGeometry();
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
BView::AllAttached()
{
}


void
BView::AllDetached()
{
}


void
BView::WindowActivated(bool state)
{
}


void
BView::MouseDown(BPoint where)
{
}


void
BView::MouseUp(BPoint where)
{
}


void
BView::MouseMoved(BPoint where, uint32 code, const BMessage* dragMessage)
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
