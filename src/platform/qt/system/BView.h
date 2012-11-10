/*
 * Copyright 2001-2009, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef PLATFORM_QT_BVIEW_H
#define PLATFORM_QT_BVIEW_H


#include <Alignment.h>
#include <Handler.h>
#include <InterfaceDefs.h>
#include <Rect.h>

#include <QWidget>


class BCursor;
class BRegion;
class BWindow;


// mouse button
enum {
	B_PRIMARY_MOUSE_BUTTON				= 0x01,
	B_SECONDARY_MOUSE_BUTTON			= 0x02,
	B_TERTIARY_MOUSE_BUTTON				= 0x04
};

// mouse transit
enum {
	B_ENTERED_VIEW						= 0,
	B_INSIDE_VIEW,
	B_EXITED_VIEW,
	B_OUTSIDE_VIEW
};

// event mask
enum {
	B_POINTER_EVENTS					= 0x00000001,
	B_KEYBOARD_EVENTS					= 0x00000002
};

// event mask options
enum {
	B_LOCK_WINDOW_FOCUS					= 0x00000001,
	B_SUSPEND_VIEW_FOCUS				= 0x00000002,
	B_NO_POINTER_HISTORY				= 0x00000004,
	// NOTE: New in Haiku (unless this flag is
	// specified, both BWindow and BView::GetMouse()
	// will filter out older mouse moved messages)
	B_FULL_POINTER_HISTORY				= 0x00000008
};

// view flags
const uint32 B_FULL_UPDATE_ON_RESIZE 	= 0x80000000UL;	/* 31 */
const uint32 _B_RESERVED1_ 				= 0x40000000UL;	/* 30 */
const uint32 B_WILL_DRAW 				= 0x20000000UL;	/* 29 */
const uint32 B_PULSE_NEEDED 			= 0x10000000UL;	/* 28 */
const uint32 B_NAVIGABLE_JUMP 			= 0x08000000UL;	/* 27 */
const uint32 B_FRAME_EVENTS				= 0x04000000UL;	/* 26 */
const uint32 B_NAVIGABLE 				= 0x02000000UL;	/* 25 */
const uint32 B_SUBPIXEL_PRECISE 		= 0x01000000UL;	/* 24 */
const uint32 B_DRAW_ON_CHILDREN 		= 0x00800000UL;	/* 23 */
const uint32 B_INPUT_METHOD_AWARE 		= 0x00400000UL;	/* 23 */
const uint32 _B_RESERVED7_ 				= 0x00200000UL;	/* 22 */
const uint32 B_SUPPORTS_LAYOUT			= 0x00100000UL;	/* 21 */
const uint32 B_INVALIDATE_AFTER_LAYOUT	= 0x00080000UL;	/* 20 */

#define _RESIZE_MASK_ (0xffff)

const uint32 _VIEW_TOP_				 	= 1UL;
const uint32 _VIEW_LEFT_ 				= 2UL;
const uint32 _VIEW_BOTTOM_			 	= 3UL;
const uint32 _VIEW_RIGHT_ 				= 4UL;
const uint32 _VIEW_CENTER_ 				= 5UL;

inline uint32 _rule_(uint32 r1, uint32 r2, uint32 r3, uint32 r4)
	{ return ((r1 << 12) | (r2 << 8) | (r3 << 4) | r4); }

#define B_FOLLOW_NONE 0
#define B_FOLLOW_ALL_SIDES	_rule_(_VIEW_TOP_, _VIEW_LEFT_, _VIEW_BOTTOM_, \
								_VIEW_RIGHT_)
#define B_FOLLOW_ALL  		B_FOLLOW_ALL_SIDES

#define B_FOLLOW_LEFT		_rule_(0, _VIEW_LEFT_, 0, _VIEW_LEFT_)
#define B_FOLLOW_RIGHT		_rule_(0, _VIEW_RIGHT_, 0, _VIEW_RIGHT_)
#define B_FOLLOW_LEFT_RIGHT	_rule_(0, _VIEW_LEFT_, 0, _VIEW_RIGHT_)
#define B_FOLLOW_H_CENTER	_rule_(0, _VIEW_CENTER_, 0, _VIEW_CENTER_)

#define B_FOLLOW_TOP		_rule_(_VIEW_TOP_, 0, _VIEW_TOP_, 0)
#define B_FOLLOW_BOTTOM		_rule_(_VIEW_BOTTOM_, 0, _VIEW_BOTTOM_, 0)
#define B_FOLLOW_TOP_BOTTOM	_rule_(_VIEW_TOP_, 0, _VIEW_BOTTOM_, 0)
#define B_FOLLOW_V_CENTER	_rule_(_VIEW_CENTER_, 0, _VIEW_CENTER_, 0)


class BView : public QWidget, public BHandler {
public:
								BView(BMessage* archive);
								BView(const char* name, uint32 flags);
								BView(BRect frame, const char* name,
									uint32 resizeMask, uint32 flags);
	virtual						~BView();

	virtual	void				MessageReceived(BMessage* message);

			BWindow*			Window() const
									{ return fWindow; }

			BRect				Bounds() const;

			uint32				Flags() const;
	virtual	void				SetFlags(uint32 flags);

			void				Invalidate(BRect invalRect);
			void				Invalidate(const BRegion* invalRegion);
			void				Invalidate();
			void				InvalidateLayout(bool descendants = false);

			void				MoveBy(float dh, float dv)
									{ move(x() + int(dh), y() + int(dv)); }
			void				MoveTo(float x, float y)
									{ move(int(x), int(y)); }
			void				MoveTo(BPoint where)
									{ MoveTo(where.x, where.y); }
			void				ResizeBy(float dh, float dv)
									{ resize(width() + int(dh), height() + int(dv)); }
			void				ResizeTo(float width, float height)
									{ resize(int(width) + 1, int(height) + 1); }
			void				ResizeTo(BSize size)
									{ ResizeTo(size.width, size.height); }

	virtual	void				MakeFocus(bool focusState = true);
			bool				IsFocus() const
									{ return hasFocus(); }

			void				ConvertToScreen(BPoint* pt) const
									{ *pt = ConvertToScreen(*pt); }
			BPoint				ConvertToScreen(BPoint pt) const;
			void				ConvertFromScreen(BPoint* pt) const
									{ *pt = ConvertFromScreen(*pt); }
			BPoint				ConvertFromScreen(BPoint pt) const;

			void				SetViewCursor(const BCursor* cursor,
									bool sync = true);

	virtual	void				SetViewColor(rgb_color c);
			void				SetViewColor(uchar r, uchar g, uchar b,
									uchar a = 255)
									{ SetViewColor(make_color(r, g, b, a)); }

			status_t			SetMouseEventMask(uint32 mask,
									uint32 options = 0);

			void				GetMouse(BPoint* _location, uint32* _buttons,
									bool checkMessageQueue = true);

	virtual	void				AttachedToWindow();
	virtual	void				DetachedFromWindow();
	virtual	void				AllAttached();
	virtual	void				AllDetached();

	virtual	void				WindowActivated(bool state);

	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 code,
									const BMessage* dragMessage);

	virtual	void				KeyDown(const char* bytes, int32 numBytes);
	virtual	void				KeyUp(const char* bytes, int32 numBytes);

	virtual	void				Pulse();
	virtual	void				FrameMoved(BPoint newPosition);
	virtual	void				FrameResized(float newWidth, float newHeight);

	// layout related

	virtual	BSize				MinSize();
	virtual	BSize				MaxSize();
	virtual	BSize				PreferredSize();
	virtual	BAlignment			LayoutAlignment();

			void				SetExplicitMinSize(BSize size);
			void				SetExplicitMaxSize(BSize size);
			void				SetExplicitPreferredSize(BSize size);
			void				SetExplicitAlignment(BAlignment alignment);

			BSize				ExplicitMinSize() const;
			BSize				ExplicitMaxSize() const;
			BSize				ExplicitPreferredSize() const;
			BAlignment			ExplicitAlignment() const;

	virtual	bool				HasHeightForWidth();
	virtual	void				GetHeightForWidth(float width, float* min,
									float* max, float* preferred);

	static	int32				FromQtMouseButtons(Qt::MouseButtons  qtButtons);
	static	int32				FromQtModifiers(
									Qt::KeyboardModifiers qtModifiers);

	virtual	QSize				minimumSizeHint() const;
	virtual	QSize				sizeHint() const;

protected:
	virtual	void				mousePressEvent(QMouseEvent* event);
	virtual	void				mouseReleaseEvent(QMouseEvent* event);
	virtual	void				mouseMoveEvent(QMouseEvent* event);
	virtual	void				leaveEvent(QEvent* event);
	virtual	void				tabletEvent(QTabletEvent* event);
	virtual	void				wheelEvent(QWheelEvent* event);

	virtual	void				keyPressEvent(QKeyEvent* event);
	virtual	void				keyReleaseEvent(QKeyEvent* event);

	virtual void				moveEvent(QMoveEvent* event);
	virtual void				resizeEvent(QResizeEvent* event);

private:
			friend class BWindow;

private:
			void				_AttachToWindow(BWindow* window);
			void				_DetachFromWindow();
			void				_AllAttachedToWindow();
			void				_AllDetachedFromWindow();

			void				_TranslateMouseEvent(QMouseEvent& event,
									BMessage& message);
			void				_TranslateTabletEvent(QTabletEvent& event,
									BMessage& message);
			template<typename Event>
			void				_TranslatePointerDeviceEvent(Event& event,
									BMessage& message);
			void				_TranslateKeyEvent(QKeyEvent& event,
									BMessage& message, bool& _isModifier);

			void				_DeliverMessage(BMessage* message);

private:
			BWindow*			fWindow;

			BSize				fMinSize;
			BSize				fMaxSize;
			BSize				fPreferredSize;
			BAlignment			fAlignment;

			BPoint				fMousePosition;
			int32				fMouseButtons;

			bool				fMouseInsideView;
			bool				fEventMessageWasHandled;
};


#endif // PLATFORM_QT_BVIEW_H
