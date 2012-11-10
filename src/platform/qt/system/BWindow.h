/*
 * Copyright 2001-2011, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef BWINDOW_H
#define BWINDOW_H


#include <InterfaceDefs.h>
#include <Looper.h>

#include <QMainWindow>


class BMessage;
class BMessageFilter;


enum window_type {
	B_UNTYPED_WINDOW					= 0,
	B_TITLED_WINDOW 					= 1,
	B_MODAL_WINDOW 						= 3,
	B_DOCUMENT_WINDOW					= 11,
	B_BORDERED_WINDOW					= 20,
	B_FLOATING_WINDOW					= 21
};

enum window_look {
	B_BORDERED_WINDOW_LOOK				= 20,
	B_NO_BORDER_WINDOW_LOOK				= 19,
	B_TITLED_WINDOW_LOOK				= 1,
	B_DOCUMENT_WINDOW_LOOK				= 11,
	B_MODAL_WINDOW_LOOK					= 3,
	B_FLOATING_WINDOW_LOOK				= 7
};

enum window_feel {
	B_NORMAL_WINDOW_FEEL				= 0,
	B_MODAL_SUBSET_WINDOW_FEEL			= 2,
	B_MODAL_APP_WINDOW_FEEL				= 1,
	B_MODAL_ALL_WINDOW_FEEL				= 3,
	B_FLOATING_SUBSET_WINDOW_FEEL		= 5,
	B_FLOATING_APP_WINDOW_FEEL			= 4,
	B_FLOATING_ALL_WINDOW_FEEL			= 6
};

enum window_alignment {
	B_BYTE_ALIGNMENT	= 0,
	B_PIXEL_ALIGNMENT	= 1
};

// window flags
enum {
	B_NOT_MOVABLE						= 0x00000001,
	B_NOT_CLOSABLE						= 0x00000020,
	B_NOT_ZOOMABLE						= 0x00000040,
	B_NOT_MINIMIZABLE					= 0x00004000,
	B_NOT_RESIZABLE						= 0x00000002,
	B_NOT_H_RESIZABLE					= 0x00000004,
	B_NOT_V_RESIZABLE					= 0x00000008,
	B_AVOID_FRONT						= 0x00000080,
	B_AVOID_FOCUS						= 0x00002000,
	B_WILL_ACCEPT_FIRST_CLICK			= 0x00000010,
	B_OUTLINE_RESIZE					= 0x00001000,
	B_NO_WORKSPACE_ACTIVATION			= 0x00000100,
	B_NOT_ANCHORED_ON_ACTIVATE			= 0x00020000,
	B_ASYNCHRONOUS_CONTROLS				= 0x00080000,
	B_QUIT_ON_WINDOW_CLOSE				= 0x00100000,
	B_SAME_POSITION_IN_ALL_WORKSPACES	= 0x00200000,
	B_AUTO_UPDATE_SIZE_LIMITS			= 0x00400000,
	B_CLOSE_ON_ESCAPE					= 0x00800000,
	B_NO_SERVER_SIDE_WINDOW_MODIFIERS	= 0x00000200
};

#define B_CURRENT_WORKSPACE				0
#define B_ALL_WORKSPACES				0xffffffff


class BWindow : public QMainWindow, public BLooper {
	Q_OBJECT

public:
								BWindow(BRect frame, const char* title,
									window_type type, uint32 flags,
									uint32 workspace = B_CURRENT_WORKSPACE);
								BWindow(BRect frame, const char* title,
									window_look look, window_feel feel,
									uint32 flags, uint32 workspace
										= B_CURRENT_WORKSPACE);
	virtual						~BWindow();

			BRect				Bounds() const
									{ return BRect::FromQRect(rect()); }
			BRect				Frame() const
									{ return BRect::FromQRect(geometry()); }
			BSize				Size() const
									{ return Bounds().Size(); }

			void				SetTitle(const char* title);

			bool				IsActive() const
									{ return isActiveWindow(); }
			void				Activate(bool active = true);

			void				MoveTo(BPoint position)
									{ MoveTo(position.x, position.y); }
			void				MoveTo(float x, float y)
									{ move((int)x, (int)y); }

			void 				CenterIn(const BRect& rect);
			void 				CenterOnScreen();

	virtual	void				Show();
	virtual	void				Hide();

	virtual	void				DispatchMessage(BMessage* message,
									BHandler* handler);

signals:
			void				PlatformWindowClosing();

protected:
			void				closeEvent(QCloseEvent* event);

private:
			struct ViewAncestryTracker;

private:
			void				_WidgetAdded(QWidget* widget);
			void				_WidgetRemoved(QWidget* widget);

private:
			int32				fShowLevel;
};


#endif // BWINDOW_H
