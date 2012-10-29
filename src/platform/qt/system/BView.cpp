#include "BView.h"

#include <Region.h>
#include <Screen.h>
#include <Window.h>

#include <MessagePrivate.h>

#include <QApplication>
#include <QEvent>
#include <QMouseEvent>


BView::BView(BMessage* archive)
	:
	QWidget(NULL),
	BHandler(archive),
	fWindow(NULL)
{
}


BView::BView(const char* name, uint32 flags)
	:
	QWidget(NULL),
	BHandler(name),
	fWindow(NULL)
{
}


BView::BView(BRect frame, const char* name, uint32 resizeMask, uint32 flags)
	:
	QWidget(NULL),
	BHandler(name),
	fWindow(NULL)
{
}


BView::~BView()
{
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


int32
BView::FromQtModifiers(Qt::KeyboardModifiers qtModifiers)
{
	// TODO: We assume Control == COMMAND, Alt == Control (i.e. standard Haiku
	// mapping), and Meta == Option.
	// TODO: We don't know whether the left or the right modifier was used and
	// assume left.
	int32 modifiers = 0;
	if ((qtModifiers & Qt::ShiftModifier) != 0)
		modifiers = B_SHIFT_KEY | B_LEFT_SHIFT_KEY;
	if ((qtModifiers & Qt::ControlModifier) != 0)
		modifiers = B_COMMAND_KEY | B_LEFT_COMMAND_KEY;
	if ((qtModifiers & Qt::AltModifier) != 0)
		modifiers = B_CONTROL_KEY | B_LEFT_CONTROL_KEY;
	if ((qtModifiers & Qt::MetaModifier) != 0)
		modifiers = B_OPTION_KEY | B_LEFT_OPTION_KEY;
	return modifiers;
}


void
BView::mousePressEvent(QMouseEvent* event)
{
	if (Looper() != NULL) {
		BMessage message(B_MOUSE_DOWN);
		_TranslateMouseEvent(*event, message);
		_DeliverMessage(&message);
		return;
	}

	QWidget::mousePressEvent(event);
}


void
BView::mouseReleaseEvent(QMouseEvent* event)
{
	if (Looper() != NULL) {
		BMessage message(B_MOUSE_UP);
		_TranslateMouseEvent(*event, message);
		_DeliverMessage(&message);
		return;
	}

	QWidget::mouseReleaseEvent(event);
}


void
BView::mouseMoveEvent(QMouseEvent* event)
{
	if (Looper() != NULL) {
		BMessage message(B_MOUSE_MOVED);
		_TranslateMouseEvent(*event, message);
		_DeliverMessage(&message);
		return;
	}

	QWidget::mouseMoveEvent(event);
}


void
BView::tabletEvent(QTabletEvent* event)
{
	uint32 messageWhat = 0;
	switch (event->type()) {
		case QEvent::TabletMove:
			messageWhat = B_MOUSE_MOVED;
			break;
		case QEvent::TabletPress:
			messageWhat = B_MOUSE_DOWN;
			break;
		case QEvent::TabletRelease:
			messageWhat = B_MOUSE_UP;
			break;
		default:
			break;
	}

	if (Looper() != NULL && messageWhat != 0) {
		BMessage message(messageWhat);
		_TranslateTabletEvent(*event, message);
		_DeliverMessage(&message);
		event->accept();
			// need to accept() explicitly according to docs
		return;
	}

	QWidget::tabletEvent(event);
}


void
BView::_AttachToWindow(BWindow* window)
{
	fWindow = window;
	fWindow->AddHandler(this);

	AttachedToWindow();
}


void
BView::_DetachFromWindow()
{
	DetachedFromWindow();
}


void
BView::_AllAttachedToWindow()
{
	AllAttached();
}


void
BView::_AllDetachedFromWindow()
{
	AllDetached();

	fWindow->RemoveHandler(this);
	fWindow = NULL;
}


void
BView::_TranslateMouseEvent(QMouseEvent& event, BMessage& message)
{
	_TranslatePointerDeviceEvent(event, message);

	// mouse buttons
	Qt::MouseButtons qtButtons = event.buttons();
	int32 buttons = 0;
	if ((qtButtons & Qt::LeftButton) != 0)
		buttons |= B_PRIMARY_MOUSE_BUTTON;
	if ((qtButtons & Qt::RightButton) != 0)
		buttons |= B_SECONDARY_MOUSE_BUTTON;
	if ((qtButtons & Qt::MiddleButton) != 0)
		buttons |= B_TERTIARY_MOUSE_BUTTON;
	message.AddInt32("buttons", buttons);

	// mouse clicks
	// TODO: int32 "clicks"!
}


void
BView::_TranslateTabletEvent(QTabletEvent& event, BMessage& message)
{
	_TranslatePointerDeviceEvent(event, message);

	// pressure
	message.AddFloat("be:tablet_pressure", event.pressure());

	// tilt
	message.AddFloat("be:tablet_tilt_x", (float)event.xTilt() / 180 * M_PI);
	message.AddFloat("be:tablet_tilt_y", (float)event.yTilt() / 180 * M_PI);

	// relative positions
	BSize screenSize = BScreen().Frame().Size();
	message.AddFloat("be:tablet_x", event.hiResGlobalX() / screenSize.Width());
	message.AddFloat("be:tablet_y", event.hiResGlobalY() / screenSize.Height());
}


template<typename Event>
void
BView::_TranslatePointerDeviceEvent(Event& event, BMessage& message)
{
	// event time
	bigtime_t eventTime = system_time();
		// TODO: Event timestamps are available with Qt 5.
	message.AddInt64("when", eventTime);

	// mouse position
	message.AddPoint("be:view_where", BPoint::FromQPoint(event.pos()));
	message.AddPoint("screen_where", BPoint::FromQPoint(event.globalPos()));

	// keyboard modifiers
	message.AddInt32("modifiers", FromQtModifiers(event.modifiers()));
}


void
BView::_DeliverMessage(BMessage* message)
{
	BMessage::Private(message).GetMessageHeader()->target = Token();
	Looper()->_DispatchMessage(message);
}
