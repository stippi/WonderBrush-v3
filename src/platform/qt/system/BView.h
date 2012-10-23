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


class BView : public PlatformWidgetHandler<QWidget>
{
public:
								BView(const char* name, uint32 flags);
								BView(BRect frame, const char* name,
									uint32 resizeMask, uint32 flags);
	virtual						~BView();

			BWindow*			Window() const;

			void				Invalidate(BRect invalRect);
			void				Invalidate(const BRegion* invalRegion);
			void				Invalidate();

			void				ConvertToScreen(BPoint* pt) const
									{ *pt = ConvertToScreen(*pt); }
			BPoint				ConvertToScreen(BPoint pt) const;
			void				ConvertFromScreen(BPoint* pt) const
									{ *pt = ConvertFromScreen(*pt); }
			BPoint				ConvertFromScreen(BPoint pt) const;

			void				SetViewCursor(const BCursor* cursor,
									bool sync = true);

			status_t			SetMouseEventMask(uint32 mask,
									uint32 options = 0);

	virtual	void				AttachedToWindow();
	virtual	void				DetachedFromWindow();

	virtual	void				KeyDown(const char* bytes, int32 numBytes);
	virtual	void				KeyUp(const char* bytes, int32 numBytes);
};


#endif // PLATFORM_QT_BVIEW_H
