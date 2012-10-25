/*
 * Copyright 2001-2009, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef PLATFORM_QT_BVIEW_H
#define PLATFORM_QT_BVIEW_H


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


class BView : public PlatformWidgetHandler<QWidget>
{
public:
								BView(BMessage* archive);
								BView(const char* name, uint32 flags);
								BView(BRect frame, const char* name,
									uint32 resizeMask, uint32 flags);
	virtual						~BView();

			BWindow*			Window() const;

			BRect				Bounds() const;

			uint32				Flags() const;
	virtual	void				SetFlags(uint32 flags);

			void				Invalidate(BRect invalRect);
			void				Invalidate(const BRegion* invalRegion);
			void				Invalidate();
			void				InvalidateLayout(bool descendants = false);

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

			void				SetExplicitMaxSize(BSize size);

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
};


#endif // PLATFORM_QT_BVIEW_H
