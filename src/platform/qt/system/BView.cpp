#include "BView.h"

#include <Bitmap.h>
#include <Cursor.h>
#include <LayoutUtils.h>
#include <Region.h>
#include <Screen.h>
#include <Window.h>

#include <MessagePrivate.h>

#include <QApplication>
#include <QEvent>
#include <QLayout>
#include <QMouseEvent>


BView::BView(BMessage* archive)
	:
	QWidget(NULL),
	BHandler(archive),
	fWindow(NULL),
	fMinSize(),
	fMaxSize(),
	fPreferredSize(),
	fAlignment(),
	fMousePosition(0, 0),
	fMouseButtons(0),
	fEventMessageWasHandled(false)
{
	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);
}


BView::BView(const char* name, uint32 flags)
	:
	QWidget(NULL),
	BHandler(name),
	fWindow(NULL),
	fMinSize(),
	fMaxSize(),
	fPreferredSize(),
	fAlignment(),
	fMousePosition(0, 0),
	fMouseButtons(0),
	fMouseInsideView(false),
	fEventMessageWasHandled(false)
{
	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);
}


BView::BView(BRect frame, const char* name, uint32 resizeMask, uint32 flags)
	:
	QWidget(NULL),
	BHandler(name),
	fWindow(NULL),
	fMinSize(),
	fMaxSize(),
	fPreferredSize(),
	fAlignment(),
	fMousePosition(0, 0),
	fMouseButtons(0),
	fEventMessageWasHandled(false)
{
	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);

	ResizeTo(frame.Size());
}


BView::~BView()
{
}


void
BView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_UNMAPPED_KEY_DOWN:
		case B_UNMAPPED_KEY_UP:
		case B_MOUSE_WHEEL_CHANGED:
			fEventMessageWasHandled = false;
			break;

		default:
			BHandler::MessageReceived(message);
	}
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
	// If there's no layout set on the widget, set the maximum size. Otherwise
	// don't interfere with Qt's layout mechanism.
	if (layout() == NULL) {
		BSize size = MaxSize();
		setMaximumSize(std::min(int(size.width) + 1, QWIDGETSIZE_MAX),
			std::min(int(size.height) + 1, QWIDGETSIZE_MAX));
	}

	updateGeometry();
	// TODO: Handle descendants!
}


void
BView::MakeFocus(bool focusState)
{
	if (focusState)
		setFocus();
	// NOTE: With Qt we can't really unfocus.
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
	if (cursor != NULL)
		setCursor(cursor->GetQCursor());
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
BView::GetMouse(BPoint* _location, uint32* _buttons, bool checkMessageQueue)
{
	if (_location == NULL && _buttons == NULL)
		return;

	if (checkMessageQueue) {
		if (_location != NULL)
			*_location = BPoint::FromQPoint(mapFromGlobal(QCursor::pos()));
		if (_buttons != NULL)
			*_buttons = FromQtMouseButtons(QApplication::mouseButtons());
	} else {
		if (_location != NULL)
			*_location = fMousePosition;
		if (_buttons != NULL)
			*_buttons = fMouseButtons;
	}
}


void
BView::DragMessage(BMessage* message, BRect dragRect, BHandler* replyTo)
{
	if (!message)
		return;

//	_CheckOwnerLock();

	// calculate the offset
	BPoint offset;
	uint32 buttons;
	BMessage* current = Window()->CurrentMessage();
	if (!current || current->FindPoint("be:view_where", &offset) != B_OK)
		GetMouse(&offset, &buttons, false);
	offset -= dragRect.LeftTop();

	if (!dragRect.IsValid()) {
		DragMessage(message, NULL, B_OP_BLEND, offset, replyTo);
		return;
	}

	// TODO: that's not really what should happen - the app_server should take
	// the chance *NOT* to need to drag a whole bitmap around but just a frame.

	// create a drag bitmap for the rect
	BBitmap* bitmap = new(std::nothrow) BBitmap(dragRect, B_RGBA32);
	if (bitmap == NULL)
		return;

	uint32* bits = (uint32*)bitmap->Bits();
	uint32 bytesPerRow = bitmap->BytesPerRow();
	uint32 width = dragRect.IntegerWidth() + 1;
	uint32 height = dragRect.IntegerHeight() + 1;
	uint32 lastRow = (height - 1) * width;

	memset(bits, 0x00, height * bytesPerRow);

	// top
	for (uint32 i = 0; i < width; i += 2)
		bits[i] = 0xff000000;

	// bottom
	for (uint32 i = (height % 2 == 0 ? 1 : 0); i < width; i += 2)
		bits[lastRow + i] = 0xff000000;

	// left
	for (uint32 i = 0; i < lastRow; i += width * 2)
		bits[i] = 0xff000000;

	// right
	for (uint32 i = (width % 2 == 0 ? width : 0); i < lastRow; i += width * 2)
		bits[width - 1 + i] = 0xff000000;

	DragMessage(message, bitmap, B_OP_BLEND, offset, replyTo);
}


void
BView::DragMessage(BMessage* message, BBitmap* image, BPoint offset,
	BHandler* replyTo)
{
	DragMessage(message, image, B_OP_COPY, offset, replyTo);
}


void
BView::DragMessage(BMessage* message, BBitmap* image,
	drawing_mode dragMode, BPoint offset, BHandler* replyTo)
{
	if (message == NULL)
		return;

	if (image == NULL) {
		// TODO: workaround for drags without a bitmap - should not be necessary if
		//	we move the rectangle dragging into the app_server
		image = new(std::nothrow) BBitmap(BRect(0, 0, 0, 0), B_RGBA32);
		if (image == NULL)
			return;
	}

	if (replyTo == NULL)
		replyTo = this;

	if (replyTo->Looper() == NULL)
		debugger("DragMessage: warning - the Handler needs a looper");

//	_CheckOwnerLock();

	if (!message->HasInt32("buttons")) {
		BMessage* msg = Window()->CurrentMessage();
		uint32 buttons;

		if (msg == NULL
			|| msg->FindInt32("buttons", (int32*)&buttons) != B_OK) {
			BPoint point;
			GetMouse(&point, &buttons, false);
		}

		message->AddInt32("buttons", buttons);
	}

#if 0
	BMessage::Private privateMessage(message);
	privateMessage.SetReply(BMessenger(replyTo, replyTo->Looper()));

	int32 bufferSize = message->FlattenedSize();
	char* buffer = new(std::nothrow) char[bufferSize];
	if (buffer != NULL) {
		message->Flatten(buffer, bufferSize);

		fOwner->fLink->StartMessage(AS_VIEW_DRAG_IMAGE);
		fOwner->fLink->Attach<int32>(image->_ServerToken());
		fOwner->fLink->Attach<int32>((int32)dragMode);
		fOwner->fLink->Attach<BPoint>(offset);
		fOwner->fLink->Attach<int32>(bufferSize);
		fOwner->fLink->Attach(buffer, bufferSize);

		// we need to wait for the server
		// to actually process this message
		// before we can delete the bitmap
		int32 code;
		fOwner->fLink->FlushWithReply(code);

		delete [] buffer;
	} else {
		fprintf(stderr, "BView::DragMessage() - no memory to flatten drag "
			"message\n");
	}
#endif
// TODO:...

	delete image;
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
	fEventMessageWasHandled = false;
}


void
BView::KeyUp(const char* bytes, int32 numBytes)
{
	fEventMessageWasHandled = false;
}


void
BView::Pulse()
{
}


void
BView::FrameMoved(BPoint newPosition)
{
}


void
BView::FrameResized(float newWidth, float newHeight)
{
}


BSize
BView::MinSize()
{
	BSize layoutSize(0, 0);

	if (QLayout* layout = this->layout()) {
		QSize size = layout->minimumSize();
		layoutSize.width = size.width() - 1;
		layoutSize.height = size.width() - 1;
	}

	return BLayoutUtils::ComposeSize(fMinSize, layoutSize);
}


BSize
BView::MaxSize()
{
	BSize layoutSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED);

	if (QLayout* layout = this->layout()) {
		QSize size = layout->maximumSize();
		layoutSize.width = size.width() - 1;
		layoutSize.height = size.width() - 1;
	}

	return BLayoutUtils::ComposeSize(fMaxSize, layoutSize);
}


BSize
BView::PreferredSize()
{
	BSize layoutSize(0, 0);

	if (QLayout* layout = this->layout()) {
		QSize size = layout->sizeHint();
		layoutSize.width = size.width() - 1;
		layoutSize.height = size.width() - 1;
	}

	return BLayoutUtils::ComposeSize(fPreferredSize, layoutSize);
}


BAlignment
BView::LayoutAlignment()
{
	return BLayoutUtils::ComposeAlignment(fAlignment, BAlignment(B_ALIGN_HORIZONTAL_CENTER, B_ALIGN_VERTICAL_CENTER));
}


void
BView::SetExplicitMinSize(BSize size)
{
	fMinSize = size;
	InvalidateLayout();
}


void
BView::SetExplicitMaxSize(BSize size)
{
	fMaxSize = size;
	InvalidateLayout();
}


void
BView::SetExplicitPreferredSize(BSize size)
{
	fPreferredSize = size;
	InvalidateLayout();
}


void
BView::SetExplicitAlignment(BAlignment alignment)
{
	fAlignment = alignment;
	InvalidateLayout();
}


BSize
BView::ExplicitMinSize() const
{
	return fMinSize;
}


BSize
BView::ExplicitMaxSize() const
{
	return fMaxSize;
}


BSize
BView::ExplicitPreferredSize() const
{
	return fPreferredSize;
}


BAlignment
BView::ExplicitAlignment() const
{
	return fAlignment;
}


bool
BView::HasHeightForWidth()
{
	return false;
}


void
BView::GetHeightForWidth(float width, float* min, float* max, float* preferred)
{
}


int32
BView::FromQtMouseButtons(Qt::MouseButtons qtButtons)
{
	int32 buttons = 0;
	if ((qtButtons & Qt::LeftButton) != 0)
		buttons |= B_PRIMARY_MOUSE_BUTTON;
	if ((qtButtons & Qt::RightButton) != 0)
		buttons |= B_SECONDARY_MOUSE_BUTTON;
	if ((qtButtons & Qt::MiddleButton) != 0)
		buttons |= B_TERTIARY_MOUSE_BUTTON;
	return buttons;
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


QSize
BView::minimumSizeHint() const
{
	// Don't interfere with Qt's layout mechanism, if a layout has been set on
	// the widget.
	if (layout() != NULL)
		return QWidget::minimumSizeHint();

	BSize size = const_cast<BView*>(this)->MinSize();
	QSize qSize(std::min(int(size.width) + 1, QWIDGETSIZE_MAX),
		std::min(int(size.height) + 1, QWIDGETSIZE_MAX));
	return qSize;
}


QSize
BView::sizeHint() const
{
	// Don't interfere with Qt's layout mechanism, if a layout has been set on
	// the widget.
	if (layout() != NULL)
		return QWidget::sizeHint();

	BSize size = const_cast<BView*>(this)->PreferredSize();
	QSize qSize(std::min(int(size.width) + 1, QWIDGETSIZE_MAX),
		std::min(int(size.height) + 1, QWIDGETSIZE_MAX));
	return qSize;
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
BView::leaveEvent(QEvent* event)
{
	if (!fMouseInsideView)
		return;

	// We have to fake a mouse message, since Qt sends only the leave event,
	// when the mouse leaves the view (and no button has been pressed).
	BMessage message(B_MOUSE_MOVED);
	fMouseInsideView = false;

	// event time
	bigtime_t eventTime = system_time();
		// TODO: Event timestamps are available with Qt 5.
	message.AddInt64("when", eventTime);

	// mouse position
	QPoint position = mapFromGlobal(QCursor::pos());
	if (rect().contains(position)) {
		// We can send a B_EXITED_VIEW message with a mouse position inside the
		// view, so move the position outside.
		QRect bounds = rect();
		int x = position.x();
		int y = position.y();
		int dx = std::min(x - bounds.left(), bounds.right() - x);
		int dy = std::min(y - bounds.top(), bounds.bottom() - y);
		if (dx <= dy) {
			position.setX(x - bounds.left() < bounds.right() - x
				? bounds.left() - 1 : bounds.right() + 1);

		} else {
			position.setY(y - bounds.top() < bounds.bottom() - y
				? bounds.top() - 1 : bounds.bottom() + 1);
		}
	}
	message.AddPoint("be:view_where", BPoint::FromQPoint(position));
	message.AddPoint("screen_where", BPoint::FromQPoint(mapToGlobal(position)));

	// mouse buttons
	int32 buttons = FromQtMouseButtons(QApplication::mouseButtons());
	message.AddInt32("buttons", buttons);

	// keyboard modifiers
	message.AddInt32("modifiers",
		FromQtModifiers(QApplication::keyboardModifiers()));

	// transit
	message.AddInt32("be:transit", B_EXITED_VIEW);

	_DeliverMessage(&message);

	QWidget::leaveEvent(event);
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

void BView::wheelEvent(QWheelEvent* event)
{
	BMessage message(B_MOUSE_WHEEL_CHANGED);

	// event time
	bigtime_t eventTime = system_time();
		// TODO: Event timestamps are available with Qt 5.
	message.AddInt64("when", eventTime);

	// float "be:wheel_delta_x", "be:wheel_delta_y"
	int horizontalDelta = 0;
	int verticalDelta = 0;
	if (event->orientation() == Qt::Horizontal)
		horizontalDelta = -event->delta() / 120;
	else
		verticalDelta = -event->delta() / 120;
	message.AddFloat("be:wheel_delta_x", horizontalDelta);
	message.AddFloat("be:wheel_delta_y", verticalDelta);

	// deliver message
	fEventMessageWasHandled = true;
	_DeliverMessage(&message);

	if (!fEventMessageWasHandled)
		QWidget::wheelEvent(event);
}


void
BView::keyPressEvent(QKeyEvent* event)
{
	fEventMessageWasHandled = false;

	if (Looper() != NULL) {
		BMessage message(B_KEY_DOWN);
		bool isModifier = false;
		_TranslateKeyEvent(*event, message, isModifier);

		if (message.HasString("bytes")) {
			fEventMessageWasHandled = true;
			_DeliverMessage(&message);
		} else {
			if (isModifier) {
				BMessage modifierMessage(message);
				modifierMessage.what = B_MODIFIERS_CHANGED;
				_DeliverMessage(&modifierMessage);
			}

			message.what = B_UNMAPPED_KEY_DOWN;
			fEventMessageWasHandled = true;
			_DeliverMessage(&message);
		}

		return;
	}

	if (!fEventMessageWasHandled)
		QWidget::keyPressEvent(event);
}


void
BView::keyReleaseEvent(QKeyEvent* event)
{
	fEventMessageWasHandled = false;

	if (Looper() != NULL) {
		BMessage message(B_KEY_UP);
		bool isModifier = false;
		_TranslateKeyEvent(*event, message, isModifier);

		if (message.HasString("bytes")) {
			fEventMessageWasHandled = true;
			_DeliverMessage(&message);
		} else {
			if (isModifier) {
				BMessage modifierMessage(message);
				modifierMessage.what = B_MODIFIERS_CHANGED;
				_DeliverMessage(&modifierMessage);
			}

			message.what = B_UNMAPPED_KEY_UP;
			fEventMessageWasHandled = true;
			_DeliverMessage(&message);
		}

		return;
	}

	if (!fEventMessageWasHandled)
		QWidget::keyReleaseEvent(event);
}


void
BView::moveEvent(QMoveEvent* event)
{
	FrameMoved(BPoint::FromQPoint(event->pos()));
}


void
BView::resizeEvent(QResizeEvent* event)
{
	QSize size = event->size();
	FrameResized(size.width() + 1, size.height() + 1);
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
	int32 buttons = FromQtMouseButtons(event.buttons());
	message.AddInt32("buttons", buttons);

	// mouse clicks
	// TODO: int32 "clicks"!

	// cache mouse buttons
	fMouseButtons = buttons;
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
	BPoint position = BPoint::FromQPoint(event.pos());
	message.AddPoint("be:view_where", position);
	message.AddPoint("screen_where", BPoint::FromQPoint(event.globalPos()));

	// keyboard modifiers
	message.AddInt32("modifiers", FromQtModifiers(event.modifiers()));

	// transit
	int32 transit;
	bool mouseInsideView = rect().contains(event.pos());
	if (fMouseInsideView) {
		if (mouseInsideView)
			transit = B_INSIDE_VIEW;
		else
			transit = B_EXITED_VIEW;
	} else {
		if (mouseInsideView)
			transit = B_ENTERED_VIEW;
		else
			transit = B_OUTSIDE_VIEW;
	}
	message.AddInt32("be:transit", transit);

	// cache mouse position
	fMousePosition = position;
	fMouseInsideView = mouseInsideView;
}


void
BView::_TranslateKeyEvent(QKeyEvent& event, BMessage& message,
	bool& _isModifier)
{
	_isModifier = false;

	// event time
	bigtime_t eventTime = system_time();
		// TODO: Event timestamps are available with Qt 5.
	message.AddInt64("when", eventTime);

	// keyboard modifiers
	int32 modifiers = FromQtModifiers(event.modifiers());
	message.AddInt32("modifiers", modifiers);

	// int32 "key"
	// int32 "raw_char"
	bool knownKey = true;
	int32 key = 0;
	int32 rawChar = 0;
	switch (event.key()) {
		case Qt::Key_Escape:
			key = 1;
			rawChar = B_ESCAPE;
			break;
		case Qt::Key_Tab:
			key = 0x26;
			rawChar = B_TAB;
			break;
//		case Qt::Key_Backtab:
		case Qt::Key_Backspace:
			key = 0x1e;
			rawChar = B_BACKSPACE;
			break;
		case Qt::Key_Return:
			key = 0x47;
			rawChar = B_RETURN;
			break;
		case Qt::Key_Enter:
			key = 0x5b;
			rawChar = B_ENTER;
			break;
		case Qt::Key_Insert:
			key = 0x64;
			rawChar = B_INSERT;
			break;
		case Qt::Key_Delete:
			key = 0x34;
			rawChar = B_DELETE;
			break;
		case Qt::Key_Pause:
			key = B_PAUSE_KEY;
			rawChar = B_FUNCTION_KEY;
			break;
		case Qt::Key_Print:
			key = B_PRINT_KEY;
			rawChar = B_FUNCTION_KEY;
			break;
		case Qt::Key_SysReq:
			key = 0x5d;
//			rawChar = ?;
			break;
//		case Qt::Key_Clear:
		case Qt::Key_Home:
			key = 0x20;
			rawChar = B_HOME;
			break;
		case Qt::Key_End:
			key = 0x35;
			rawChar = B_END;
			break;
		case Qt::Key_Left:
			key = 0x61;
			rawChar = B_LEFT_ARROW;
			break;
		case Qt::Key_Up:
			key = 0x57;
			rawChar = B_UP_ARROW;
			break;
		case Qt::Key_Right:
			key = 0x63;
			rawChar = B_RIGHT_ARROW;
			break;
		case Qt::Key_Down:
			key = 0x62;
			rawChar = B_DOWN_ARROW;
			break;
		case Qt::Key_PageUp:
			key = 0x21;
			rawChar = B_PAGE_UP;
			break;
		case Qt::Key_PageDown:
			key = 0x36;
			rawChar = B_PAGE_DOWN;
			break;
		case Qt::Key_Shift:
			key = 0x4b;
				// That's left shift. Right shift is 0x56.
			_isModifier = true;
			break;
		case Qt::Key_Control:
			key = 0x5c;
				// That's left control. Right control is 0x60.
			_isModifier = true;
			break;
		case Qt::Key_Meta:
			key = 0x66;
				// That's left option (Windows).
			_isModifier = true;
			break;
		case Qt::Key_Alt:
			key = 0x5d;
				// That's left alt.
			_isModifier = true;
			break;
		case Qt::Key_AltGr:
			key = 0x5f;
				// That's right alt.
			_isModifier = true;
			break;
		case Qt::Key_CapsLock:
			key = 0x3b;
			_isModifier = true;
			break;
		case Qt::Key_NumLock:
			key = 0x22;
			_isModifier = true;
			break;
		case Qt::Key_ScrollLock:
			key = 0x0f;
			_isModifier = true;
			break;
		case Qt::Key_F1:
			key = B_F1_KEY;
			rawChar = B_FUNCTION_KEY;
			break;
		case Qt::Key_F2:
			key = B_F2_KEY;
			rawChar = B_FUNCTION_KEY;
			break;
		case Qt::Key_F3:
			key = B_F3_KEY;
			rawChar = B_FUNCTION_KEY;
			break;
		case Qt::Key_F4:
			key = B_F4_KEY;
			rawChar = B_FUNCTION_KEY;
			break;
		case Qt::Key_F5:
			key = B_F5_KEY;
			rawChar = B_FUNCTION_KEY;
			break;
		case Qt::Key_F6:
			key = B_F6_KEY;
			rawChar = B_FUNCTION_KEY;
			break;
		case Qt::Key_F7:
			key = B_F7_KEY;
			rawChar = B_FUNCTION_KEY;
			break;
		case Qt::Key_F8:
			key = B_F8_KEY;
			rawChar = B_FUNCTION_KEY;
			break;
		case Qt::Key_F9:
			key = B_F9_KEY;
			rawChar = B_FUNCTION_KEY;
			break;
		case Qt::Key_F10:
			key = B_F10_KEY;
			rawChar = B_FUNCTION_KEY;
			break;
		case Qt::Key_F11:
			key = B_F11_KEY;
			rawChar = B_FUNCTION_KEY;
			break;
		case Qt::Key_F12:
			key = B_F12_KEY;
			rawChar = B_FUNCTION_KEY;
			break;
//		case Qt::Key_F13:
//		case Qt::Key_F14:
//		case Qt::Key_F15:
//		case Qt::Key_F16:
//		case Qt::Key_F17:
//		case Qt::Key_F18:
//		case Qt::Key_F19:
//		case Qt::Key_F20:
//		case Qt::Key_F21:
//		case Qt::Key_F22:
//		case Qt::Key_F23:
//		case Qt::Key_F24:
//		case Qt::Key_F25:
//		case Qt::Key_F26:
//		case Qt::Key_F27:
//		case Qt::Key_F28:
//		case Qt::Key_F29:
//		case Qt::Key_F30:
//		case Qt::Key_F31:
//		case Qt::Key_F32:
//		case Qt::Key_F33:
//		case Qt::Key_F34:
//		case Qt::Key_F35:
//		case Qt::Key_Super_L:
//		case Qt::Key_Super_R:
		case Qt::Key_Menu:
			key = 0x68;
			_isModifier = true;
			break;
//		case Qt::Key_Hyper_L:
//		case Qt::Key_Hyper_R:
//		case Qt::Key_Help:
//		case Qt::Key_Direction_L:
//		case Qt::Key_Direction_R:
		case Qt::Key_Space:
			key = 0x5e;
			rawChar = B_SPACE;
			break;
//		case Qt::Key_Any:
		case Qt::Key_Exclam:
//			key = 0x?;
			rawChar = '!';
			break;
		case Qt::Key_QuoteDbl:
//			key = 0x?;
			rawChar = '"';
			break;
		case Qt::Key_NumberSign:
//			key = 0x?;
			rawChar = '$';
			break;
		case Qt::Key_Dollar:
//			key = 0x?;
			rawChar = '$';
			break;
		case Qt::Key_Percent:
//			key = 0x?;
			rawChar = '%';
			break;
		case Qt::Key_Ampersand:
//			key = 0x?;
			rawChar = '&';
			break;
		case Qt::Key_Apostrophe:
			key = 0x46;
			rawChar = '\'';
			break;
		case Qt::Key_ParenLeft:
//			key = 0x?;
			rawChar = '(';
			break;
		case Qt::Key_ParenRight:
//			key = 0x?;
			rawChar = ')';
			break;
		case Qt::Key_Asterisk:
//			key = 0x?;
			rawChar = '*';
			break;
		case Qt::Key_Plus:
//			key = 0x?;
			rawChar = '+';
			break;
		case Qt::Key_Comma:
			key = 0x53;
			rawChar = ',';
			break;
		case Qt::Key_Minus:
			key = 0x1c;
			rawChar = '-';
			break;
		case Qt::Key_Period:
			key = 0x54;
			rawChar = '.';
			break;
		case Qt::Key_Slash:
			key = 0x55;
			rawChar = '/';
			break;
		case Qt::Key_0:
			key = 0x1b;
			rawChar = '0';
			break;
		case Qt::Key_1:
			key = 0x12;
			rawChar = '1';
			break;
		case Qt::Key_2:
			key = 0x13;
			rawChar = '2';
			break;
		case Qt::Key_3:
			key = 0x14;
			rawChar = '3';
			break;
		case Qt::Key_4:
			key = 0x15;
			rawChar = '4';
			break;
		case Qt::Key_5:
			key = 0x16;
			rawChar = '5';
			break;
		case Qt::Key_6:
			key = 0x17;
			rawChar = '6';
			break;
		case Qt::Key_7:
			key = 0x18;
			rawChar = '7';
			break;
		case Qt::Key_8:
			key = 0x19;
			rawChar = '8';
			break;
		case Qt::Key_9:
			key = 0x1a;
			rawChar = '9';
			break;
		case Qt::Key_Colon:
//			key = 0x?;
			rawChar = ':';
			break;
		case Qt::Key_Semicolon:
			key = 0x45;
			rawChar = ';';
			break;
		case Qt::Key_Less:
//			key = 0x?;
			rawChar = '<';
			break;
		case Qt::Key_Equal:
			key = 0x1d;
			rawChar = '=';
			break;
		case Qt::Key_Greater:
//			key = 0x?;
			rawChar = '>';
			break;
		case Qt::Key_Question:
//			key = 0x?;
			rawChar = '?';
			break;
		case Qt::Key_At:
//			key = 0x?;
			rawChar = '@';
			break;
		case Qt::Key_A:
			key = 0x3c;
			rawChar = 'a';
			break;
		case Qt::Key_B:
			key = 0x50;
			rawChar = 'b';
			break;
		case Qt::Key_C:
			key = 0x48;
			rawChar = 'c';
			break;
		case Qt::Key_D:
			key = 0x3e;
			rawChar = 'd';
			break;
		case Qt::Key_E:
			key = 0x29;
			rawChar = 'e';
			break;
		case Qt::Key_F:
			key = 0x3f;
			rawChar = 'f';
			break;
		case Qt::Key_G:
			key = 0x40;
			rawChar = 'g';
			break;
		case Qt::Key_H:
			key = 0x41;
			rawChar = 'h';
			break;
		case Qt::Key_I:
			key = 0x2e;
			rawChar = 'i';
			break;
		case Qt::Key_J:
			key = 0x42;
			rawChar = 'j';
			break;
		case Qt::Key_K:
			key = 0x43;
			rawChar = 'k';
			break;
		case Qt::Key_L:
			key = 0x44;
			rawChar = 'l';
			break;
		case Qt::Key_M:
			key = 0x52;
			rawChar = 'm';
			break;
		case Qt::Key_N:
			key = 0x51;
			rawChar = 'n';
			break;
		case Qt::Key_O:
			key = 0x2f;
			rawChar = 'o';
			break;
		case Qt::Key_P:
			key = 0x30;
			rawChar = 'p';
			break;
		case Qt::Key_Q:
			key = 0x27;
			rawChar = 'q';
			break;
		case Qt::Key_R:
			key = 0x2a;
			rawChar = 'r';
			break;
		case Qt::Key_S:
			key = 0x3d;
			rawChar = 's';
			break;
		case Qt::Key_T:
			key = 0x2b;
			rawChar = 't';
			break;
		case Qt::Key_U:
			key = 0x2d;
			rawChar = 'u';
			break;
		case Qt::Key_V:
			key = 0x4f;
			rawChar = 'v';
			break;
		case Qt::Key_W:
			key = 0x28;
			rawChar = 'w';
			break;
		case Qt::Key_X:
			key = 0x4d;
			rawChar = 'x';
			break;
		case Qt::Key_Y:
			key = 0x2c;
			rawChar = 'y';
			break;
		case Qt::Key_Z:
			key = 0x4c;
			rawChar = 'z';
			break;
		case Qt::Key_BracketLeft:
			key = 0x31;
			rawChar = '[';
			break;
		case Qt::Key_Backslash:
			key = 0x33;
			rawChar = '\\';
			break;
		case Qt::Key_BracketRight:
			key = 0x32;
			rawChar = ']';
			break;
		case Qt::Key_AsciiCircum:
//			key = 0x?;
			rawChar = '^';
			break;
		case Qt::Key_Underscore:
//			key = 0x?;
			rawChar = '_';
			break;
//		case Qt::Key_QuoteLeft:
		case Qt::Key_BraceLeft:
//			key = 0x?;
			rawChar = '{';
			break;
		case Qt::Key_Bar:
//			key = 0x?;
			rawChar = '|';
			break;
		case Qt::Key_BraceRight:
//			key = 0x?;
			rawChar = '}';
			break;
		case Qt::Key_AsciiTilde:
//			key = 0x?;
			rawChar = '~';
			break;
//		case Qt::Key_nobreakspace:
//		case Qt::Key_exclamdown:
//		case Qt::Key_cent:
//		case Qt::Key_sterling:
//		case Qt::Key_currency:
//		case Qt::Key_yen:
//		case Qt::Key_brokenbar:
//		case Qt::Key_section:
//		case Qt::Key_diaeresis:
//		case Qt::Key_copyright:
//		case Qt::Key_ordfeminine:
//		case Qt::Key_guillemotleft:
//		case Qt::Key_notsign:
//		case Qt::Key_hyphen:
//		case Qt::Key_registered:
//		case Qt::Key_macron:
//		case Qt::Key_degree:
//		case Qt::Key_plusminus:
//		case Qt::Key_twosuperior:
//		case Qt::Key_threesuperior:
//		case Qt::Key_acute:
//		case Qt::Key_mu:
//		case Qt::Key_paragraph:
//		case Qt::Key_periodcentered:
//		case Qt::Key_cedilla:
//		case Qt::Key_onesuperior:
//		case Qt::Key_masculine:
//		case Qt::Key_guillemotright:
//		case Qt::Key_onequarter:
//		case Qt::Key_onehalf:
//		case Qt::Key_threequarters:
//		case Qt::Key_questiondown:
//		case Qt::Key_Agrave:
//		case Qt::Key_Aacute:
//		case Qt::Key_Acircumflex:
//		case Qt::Key_Atilde:
//		case Qt::Key_Adiaeresis:
//		case Qt::Key_Aring:
//		case Qt::Key_AE:
//		case Qt::Key_Ccedilla:
//		case Qt::Key_Egrave:
//		case Qt::Key_Eacute:
//		case Qt::Key_Ecircumflex:
//		case Qt::Key_Ediaeresis:
//		case Qt::Key_Igrave:
//		case Qt::Key_Iacute:
//		case Qt::Key_Icircumflex:
//		case Qt::Key_Idiaeresis:
//		case Qt::Key_ETH:
//		case Qt::Key_Ntilde:
//		case Qt::Key_Ograve:
//		case Qt::Key_Oacute:
//		case Qt::Key_Ocircumflex:
//		case Qt::Key_Otilde:
//		case Qt::Key_Odiaeresis:
//		case Qt::Key_multiply:
//		case Qt::Key_Ooblique:
//		case Qt::Key_Ugrave:
//		case Qt::Key_Uacute:
//		case Qt::Key_Ucircumflex:
//		case Qt::Key_Udiaeresis:
//		case Qt::Key_Yacute:
//		case Qt::Key_THORN:
//		case Qt::Key_ssharp:
//		case Qt::Key_division:
//		case Qt::Key_ydiaeresis:
//		case Qt::Key_Multi_key:
//		case Qt::Key_Codeinput:
//		case Qt::Key_SingleCandidate:
//		case Qt::Key_MultipleCandidate:
//		case Qt::Key_PreviousCandidate:
//		case Qt::Key_Mode_switch:
//		case Qt::Key_Kanji:
//		case Qt::Key_Muhenkan:
//		case Qt::Key_Henkan:
//		case Qt::Key_Romaji:
//		case Qt::Key_Hiragana:
//		case Qt::Key_Katakana:
//		case Qt::Key_Hiragana_Katakana:
//		case Qt::Key_Zenkaku:
//		case Qt::Key_Hankaku:
//		case Qt::Key_Zenkaku_Hankaku:
//		case Qt::Key_Touroku:
//		case Qt::Key_Massyo:
//		case Qt::Key_Kana_Lock:
//		case Qt::Key_Kana_Shift:
//		case Qt::Key_Eisu_Shift:
//		case Qt::Key_Eisu_toggle:
//		case Qt::Key_Hangul:
//		case Qt::Key_Hangul_Start:
//		case Qt::Key_Hangul_End:
//		case Qt::Key_Hangul_Hanja:
//		case Qt::Key_Hangul_Jamo:
//		case Qt::Key_Hangul_Romaja:
//		case Qt::Key_Hangul_Jeonja:
//		case Qt::Key_Hangul_Banja:
//		case Qt::Key_Hangul_PreHanja:
//		case Qt::Key_Hangul_PostHanja:
//		case Qt::Key_Hangul_Special:
//		case Qt::Key_Dead_Grave:
//		case Qt::Key_Dead_Acute:
//		case Qt::Key_Dead_Circumflex:
//		case Qt::Key_Dead_Tilde:
//		case Qt::Key_Dead_Macron:
//		case Qt::Key_Dead_Breve:
//		case Qt::Key_Dead_Abovedot:
//		case Qt::Key_Dead_Diaeresis:
//		case Qt::Key_Dead_Abovering:
//		case Qt::Key_Dead_Doubleacute:
//		case Qt::Key_Dead_Caron:
//		case Qt::Key_Dead_Cedilla:
//		case Qt::Key_Dead_Ogonek:
//		case Qt::Key_Dead_Iota:
//		case Qt::Key_Dead_Voiced_Sound:
//		case Qt::Key_Dead_Semivoiced_Sound:
//		case Qt::Key_Dead_Belowdot:
//		case Qt::Key_Dead_Hook:
//		case Qt::Key_Dead_Horn:
//		case Qt::Key_Back:
//		case Qt::Key_Forward:
//		case Qt::Key_Stop:
//		case Qt::Key_Refresh:
//		case Qt::Key_VolumeDown:
//		case Qt::Key_VolumeMute:
//		case Qt::Key_VolumeUp:
//		case Qt::Key_BassBoost:
//		case Qt::Key_BassUp:
//		case Qt::Key_BassDown:
//		case Qt::Key_TrebleUp:
//		case Qt::Key_TrebleDown:
//		case Qt::Key_MediaPlay:
//		case Qt::Key_MediaStop:
//		case Qt::Key_MediaPrevious:
//		case Qt::Key_MediaNext:
//		case Qt::Key_MediaRecord:
//		case Qt::Key_MediaPause:
//		case Qt::Key_MediaTogglePlayPause:
//		case Qt::Key_HomePage:
//		case Qt::Key_Favorites:
//		case Qt::Key_Search:
//		case Qt::Key_Standby:
//		case Qt::Key_OpenUrl:
//		case Qt::Key_LaunchMail:
//		case Qt::Key_LaunchMedia:
//		case Qt::Key_Launch0:
//		case Qt::Key_Launch1:
//		case Qt::Key_Launch2:
//		case Qt::Key_Launch3:
//		case Qt::Key_Launch4:
//		case Qt::Key_Launch5:
//		case Qt::Key_Launch6:
//		case Qt::Key_Launch7:
//		case Qt::Key_Launch8:
//		case Qt::Key_Launch9:
//		case Qt::Key_LaunchA:
//		case Qt::Key_LaunchB:
//		case Qt::Key_LaunchC:
//		case Qt::Key_LaunchD:
//		case Qt::Key_LaunchE:
//		case Qt::Key_LaunchF:
//		case Qt::Key_LaunchG:
//		case Qt::Key_LaunchH:
//		case Qt::Key_MonBrightnessUp:
//		case Qt::Key_MonBrightnessDown:
//		case Qt::Key_KeyboardLightOnOff:
//		case Qt::Key_KeyboardBrightnessUp:
//		case Qt::Key_KeyboardBrightnessDown:
//		case Qt::Key_PowerOff:
//		case Qt::Key_WakeUp:
//		case Qt::Key_Eject:
//		case Qt::Key_ScreenSaver:
//		case Qt::Key_WWW:
//		case Qt::Key_Memo:
//		case Qt::Key_LightBulb:
//		case Qt::Key_Shop:
//		case Qt::Key_History:
//		case Qt::Key_AddFavorite:
//		case Qt::Key_HotLinks:
//		case Qt::Key_BrightnessAdjust:
//		case Qt::Key_Finance:
//		case Qt::Key_Community:
//		case Qt::Key_AudioRewind:
//		case Qt::Key_BackForward:
//		case Qt::Key_ApplicationLeft:
//		case Qt::Key_ApplicationRight:
//		case Qt::Key_Book:
//		case Qt::Key_CD:
//		case Qt::Key_Calculator:
//		case Qt::Key_ToDoList:
//		case Qt::Key_ClearGrab:
//		case Qt::Key_Close:
//		case Qt::Key_Copy:
//		case Qt::Key_Cut:
//		case Qt::Key_Display:
//		case Qt::Key_DOS:
//		case Qt::Key_Documents:
//		case Qt::Key_Excel:
//		case Qt::Key_Explorer:
//		case Qt::Key_Game:
//		case Qt::Key_Go:
//		case Qt::Key_iTouch:
//		case Qt::Key_LogOff:
//		case Qt::Key_Market:
//		case Qt::Key_Meeting:
//		case Qt::Key_MenuKB:
//		case Qt::Key_MenuPB:
//		case Qt::Key_MySites:
//		case Qt::Key_News:
//		case Qt::Key_OfficeHome:
//		case Qt::Key_Option:
//		case Qt::Key_Paste:
//		case Qt::Key_Phone:
//		case Qt::Key_Calendar:
//		case Qt::Key_Reply:
//		case Qt::Key_Reload:
//		case Qt::Key_RotateWindows:
//		case Qt::Key_RotationPB:
//		case Qt::Key_RotationKB:
//		case Qt::Key_Save:
//		case Qt::Key_Send:
//		case Qt::Key_Spell:
//		case Qt::Key_SplitScreen:
//		case Qt::Key_Support:
//		case Qt::Key_TaskPane:
//		case Qt::Key_Terminal:
//		case Qt::Key_Tools:
//		case Qt::Key_Travel:
//		case Qt::Key_Video:
//		case Qt::Key_Word:
//		case Qt::Key_Xfer:
//		case Qt::Key_ZoomIn:
//		case Qt::Key_ZoomOut:
//		case Qt::Key_Away:
//		case Qt::Key_Messenger:
//		case Qt::Key_WebCam:
//		case Qt::Key_MailForward:
//		case Qt::Key_Pictures:
//		case Qt::Key_Music:
//		case Qt::Key_Battery:
//		case Qt::Key_Bluetooth:
//		case Qt::Key_WLAN:
//		case Qt::Key_UWB:
//		case Qt::Key_AudioForward:
//		case Qt::Key_AudioRepeat:
//		case Qt::Key_AudioRandomPlay:
//		case Qt::Key_Subtitle:
//		case Qt::Key_AudioCycleTrack:
//		case Qt::Key_Time:
//		case Qt::Key_Hibernate:
//		case Qt::Key_View:
//		case Qt::Key_TopMenu:
//		case Qt::Key_PowerDown:
//		case Qt::Key_Suspend:
//		case Qt::Key_ContrastAdjust:
//		case Qt::Key_MediaLast:
//		case Qt::Key_unknown:
//		case Qt::Key_Call:
//		case Qt::Key_Camera:
//		case Qt::Key_CameraFocus:
//		case Qt::Key_Context1:
//		case Qt::Key_Context2:
//		case Qt::Key_Context3:
//		case Qt::Key_Context4:
//		case Qt::Key_Flip:
//		case Qt::Key_Hangup:
//		case Qt::Key_No:
//		case Qt::Key_Select:
//		case Qt::Key_Yes:
//		case Qt::Key_ToggleCallHangup:
//		case Qt::Key_VoiceDial:
//		case Qt::Key_LastNumberRedial:
//		case Qt::Key_Execute:
//		case Qt::Key_Printer:
//		case Qt::Key_Play:
//		case Qt::Key_Sleep:
//		case Qt::Key_Zoom:
//		case Qt::Key_Cancel:
		default:
			knownKey = false;
			break;
	}

	// If we haven't translated the key, but it has produced an ASCII char,
	// just use that as the rawChar.
	QByteArray bytes = event.text().toUtf8();
	if (!knownKey && bytes.length() == 1 && (int8)bytes.at(0) > 0)
		rawChar = bytes.at(0);

	message.AddInt32("key", key);
	if (rawChar != 0)
		message.AddInt32("raw_char", rawChar);

	// "byte" and "bytes" -- the UTF-8 individual bytes and string
	// If we have a rawChar in the ASCII range, but no bytes, use the rawChar.
	if (bytes.isEmpty() && rawChar > 0 && rawChar <= 127)
		bytes.append((char)rawChar);

	if (!bytes.isEmpty()) {
		for (int i = 0; i < bytes.length(); i++)
			message.AddInt8("byte", bytes.at(i));
		message.AddString("bytes", bytes.data());
	}

	// uint8 "states"
	// TODO: "The state of all keys at the time of the event." (BeBook)

	// int32 "be:old_modifiers"
	// NOTE: This is an approximation, since we might not get here whenever a
	// modifier changes (when no BView has focus).
	static int32 oldModifiers = 0;
	if (_isModifier)
		message.AddInt32("be:old_modifiers", oldModifiers);
	oldModifiers = modifiers;
}


void
BView::_DeliverMessage(BMessage* message)
{
	BMessage::Private(message).GetMessageHeader()->target = Token();
	Looper()->_DispatchMessage(message);
}
