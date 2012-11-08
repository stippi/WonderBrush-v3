#include "BWindow.h"

#include <typeinfo>

#include <View.h>

#include <QCoreApplication>
#include <QEvent>


struct BWindow::ViewAncestryTracker : public QObject {
	ViewAncestryTracker() :
		QObject(QCoreApplication::instance())
	{
	}

	static ViewAncestryTracker* GetTracker()
	{
		static ViewAncestryTracker* tracker = new ViewAncestryTracker;
		return tracker;
	}

	void RegisterWindow(BWindow* window)
	{
		QMutexLocker mutexLocker(&fMutex);
		bool installFilter = fWindows.isEmpty();
		fWindows.insert(window);
		if (installFilter)
			QCoreApplication::instance()->installEventFilter(this);
	}

	void UnregisterWindow(BWindow* window)
	{
		QMutexLocker mutexLocker(&fMutex);
		fWindows.remove(window);
		if (fWindows.isEmpty())
			QCoreApplication::instance()->removeEventFilter(this);
	}

protected:
	virtual bool eventFilter(QObject* watched, QEvent* event)
	{
		if (event->type() == QEvent::ParentChange) {
			if (QWidget* widget = dynamic_cast<QWidget*>(watched)) {
				QMutexLocker mutexLocker(&fMutex);
				BWindow* oldWindow = fWidgets.value(widget, NULL);
				BWindow* newWindow = dynamic_cast<BWindow*>(widget->window());
				if (oldWindow != newWindow) {
					if (newWindow == NULL)
						fWidgets.remove(widget);
					else
						fWidgets.insert(widget, newWindow);
					mutexLocker.unlock();

					if (oldWindow != NULL)
						oldWindow->_WidgetRemoved(widget);
					if (newWindow != NULL)
						newWindow->_WidgetAdded(widget);
				}
			}
		}

		return false;
	}

private:
	QMutex						fMutex;
	QSet<BWindow*>				fWindows;
	QHash<QWidget*, BWindow*>	fWidgets;
};


BWindow::BWindow(QWidget* parent)
	:
	QMainWindow(parent),
	BLooper()
{
	ObjectConstructed(this);

	ViewAncestryTracker::GetTracker()->RegisterWindow(this);
}


BWindow::~BWindow()
{
	ViewAncestryTracker::GetTracker()->RegisterWindow(this);

	ObjectAboutToBeDestroyed(this);
}


void
BWindow::DispatchMessage(BMessage* msg, BHandler* target)
{
	switch (msg->what) {
#if 0
		case B_ZOOM:
			Zoom();
			break;

		case _MINIMIZE_:
			// Used by the minimize shortcut
			if ((Flags() & B_NOT_MINIMIZABLE) == 0)
				Minimize(true);
			break;

		case _ZOOM_:
			// Used by the zoom shortcut
			if ((Flags() & B_NOT_ZOOMABLE) == 0)
				Zoom();
			break;

		case _SEND_BEHIND_:
			SendBehind(NULL);
			break;

		case _SEND_TO_FRONT_:
			Activate();
			break;

		case _SWITCH_WORKSPACE_:
		{
			int32 deltaX = 0;
			msg->FindInt32("delta_x", &deltaX);
			int32 deltaY = 0;
			msg->FindInt32("delta_y", &deltaY);
			bool takeMeThere = false;
			msg->FindBool("take_me_there", &takeMeThere);

			if (deltaX == 0 && deltaY == 0)
				break;

			BPrivate::AppServerLink link;
			link.StartMessage(AS_GET_WORKSPACE_LAYOUT);

			status_t status;
			int32 columns;
			int32 rows;
			if (link.FlushWithReply(status) != B_OK || status != B_OK)
				break;

			link.Read<int32>(&columns);
			link.Read<int32>(&rows);

			int32 current = current_workspace();

			int32 nextColumn = current % columns + deltaX;
			int32 nextRow = current / columns + deltaY;
			if (nextColumn >= columns)
				nextColumn = columns - 1;
			else if (nextColumn < 0)
				nextColumn = 0;
			if (nextRow >= rows)
				nextRow = rows - 1;
			else if (nextRow < 0)
				nextRow = 0;

			int32 next = nextColumn + nextRow * columns;
			if (next != current) {
				BPrivate::AppServerLink link;
				link.StartMessage(AS_ACTIVATE_WORKSPACE);
				link.Attach<int32>(next);
				link.Attach<bool>(takeMeThere);
				link.Flush();
			}
			break;
		}

		case B_MINIMIZE:
		{
			bool minimize;
			if (msg->FindBool("minimize", &minimize) == B_OK)
				Minimize(minimize);
			break;
		}

		case B_HIDE_APPLICATION:
		{
			// Hide all applications with the same signature
			// (ie. those that are part of the same group to be consistent
			// to what the Deskbar shows you).
			app_info info;
			be_app->GetAppInfo(&info);

			BList list;
			be_roster->GetAppList(info.signature, &list);

			for (int32 i = 0; i < list.CountItems(); i++) {
				do_minimize_team(BRect(), (team_id)list.ItemAt(i), false);
			}
			break;
		}

		case B_WINDOW_RESIZED:
		{
			int32 width, height;
			if (msg->FindInt32("width", &width) == B_OK
				&& msg->FindInt32("height", &height) == B_OK) {
				// combine with pending resize notifications
				BMessage* pendingMessage;
				while ((pendingMessage = MessageQueue()->FindMessage(B_WINDOW_RESIZED, 0))) {
					int32 nextWidth;
					if (pendingMessage->FindInt32("width", &nextWidth) == B_OK)
						width = nextWidth;

					int32 nextHeight;
					if (pendingMessage->FindInt32("height", &nextHeight) == B_OK)
						height = nextHeight;

					MessageQueue()->RemoveMessage(pendingMessage);
					delete pendingMessage;
						// this deletes the first *additional* message
						// fCurrentMessage is safe
				}
				if (width != fFrame.Width() || height != fFrame.Height()) {
					// NOTE: we might have already handled the resize
					// in an _UPDATE_ message
					fFrame.right = fFrame.left + width;
					fFrame.bottom = fFrame.top + height;

					_AdoptResize();
//					FrameResized(width, height);
				}
// call hook function anyways
// TODO: When a window is resized programmatically,
// it receives this message, and maybe it is wise to
// keep the asynchronous nature of this process to
// not risk breaking any apps.
FrameResized(width, height);
			}
			break;
		}

		case B_WINDOW_MOVED:
		{
			BPoint origin;
			if (msg->FindPoint("where", &origin) == B_OK) {
				if (fFrame.LeftTop() != origin) {
					// NOTE: we might have already handled the move
					// in an _UPDATE_ message
					fFrame.OffsetTo(origin);

//					FrameMoved(origin);
				}
// call hook function anyways
// TODO: When a window is moved programmatically,
// it receives this message, and maybe it is wise to
// keep the asynchronous nature of this process to
// not risk breaking any apps.
FrameMoved(origin);
			}
			break;
		}

		case B_WINDOW_ACTIVATED:
			if (target != this) {
				target->MessageReceived(msg);
				break;
			}

			bool active;
			if (msg->FindBool("active", &active) != B_OK)
				break;

			// find latest activation message

			while (true) {
				BMessage* pendingMessage = MessageQueue()->FindMessage(
					B_WINDOW_ACTIVATED, 0);
				if (pendingMessage == NULL)
					break;

				bool nextActive;
				if (pendingMessage->FindBool("active", &nextActive) == B_OK)
					active = nextActive;

				MessageQueue()->RemoveMessage(pendingMessage);
				delete pendingMessage;
			}

			if (active != fActive) {
				fActive = active;

				WindowActivated(active);

				// call hook function 'WindowActivated(bool)' for all
				// views attached to this window.
				fTopView->_Activate(active);

				// we notify the input server if we are gaining or losing focus
				// from a view which has the B_INPUT_METHOD_AWARE on a window
				// activation
				if (!active)
					break;
				bool inputMethodAware = false;
				if (fFocus)
					inputMethodAware = fFocus->Flags() & B_INPUT_METHOD_AWARE;
				BMessage msg(inputMethodAware ? IS_FOCUS_IM_AWARE_VIEW : IS_UNFOCUS_IM_AWARE_VIEW);
				BMessenger messenger(fFocus);
				BMessage reply;
				if (fFocus)
					msg.AddMessenger("view", messenger);
				_control_input_server_(&msg, &reply);
			}
			break;

		case B_SCREEN_CHANGED:
			if (target == this) {
				BRect frame;
				uint32 mode;
				if (msg->FindRect("frame", &frame) == B_OK
					&& msg->FindInt32("mode", (int32*)&mode) == B_OK)
					ScreenChanged(frame, (color_space)mode);
			} else
				target->MessageReceived(msg);
			break;

		case B_WORKSPACE_ACTIVATED:
			if (target == this) {
				uint32 workspace;
				bool active;
				if (msg->FindInt32("workspace", (int32*)&workspace) == B_OK
					&& msg->FindBool("active", &active) == B_OK)
					WorkspaceActivated(workspace, active);
			} else
				target->MessageReceived(msg);
			break;

		case B_WORKSPACES_CHANGED:
			if (target == this) {
				uint32 oldWorkspace, newWorkspace;
				if (msg->FindInt32("old", (int32*)&oldWorkspace) == B_OK
					&& msg->FindInt32("new", (int32*)&newWorkspace) == B_OK)
					WorkspacesChanged(oldWorkspace, newWorkspace);
			} else
				target->MessageReceived(msg);
			break;

		case B_INVALIDATE:
		{
			if (BView* view = dynamic_cast<BView*>(target)) {
				BRect rect;
				if (msg->FindRect("be:area", &rect) == B_OK)
					view->Invalidate(rect);
				else
					view->Invalidate();
			} else
				target->MessageReceived(msg);
			break;
		}
#endif

		case B_KEY_DOWN:
		{
//			if (!_HandleKeyDown(msg)) {
				if (BView* view = dynamic_cast<BView*>(target)) {
					// TODO: cannot use "string" here if we support having
					// different font encoding per view (it's supposed to be
					// converted by _HandleKeyDown() one day)
					const char* string;
					ssize_t bytes;
					if (msg->FindData("bytes", B_STRING_TYPE,
						(const void**)&string, &bytes) == B_OK) {
						view->KeyDown(string, bytes - 1);
					}
				} else
					target->MessageReceived(msg);
//			}
			break;
		}

		case B_KEY_UP:
		{
			// TODO: same as above
			if (BView* view = dynamic_cast<BView*>(target)) {
				const char* string;
				ssize_t bytes;
				if (msg->FindData("bytes", B_STRING_TYPE,
					(const void**)&string, &bytes) == B_OK) {
					view->KeyUp(string, bytes - 1);
				}
			} else
				target->MessageReceived(msg);
			break;
		}

		case B_UNMAPPED_KEY_DOWN:
		{
//			if (!_HandleUnmappedKeyDown(msg))
				target->MessageReceived(msg);
			break;
		}

		case B_MOUSE_DOWN:
		{
			BView* view = dynamic_cast<BView*>(target);

			if (view != NULL) {
				BPoint where;
				msg->FindPoint("be:view_where", &where);
				view->MouseDown(where);
			} else
				target->MessageReceived(msg);

			break;
		}

		case B_MOUSE_UP:
		{
			if (BView* view = dynamic_cast<BView*>(target)) {
				BPoint where;
				msg->FindPoint("be:view_where", &where);
// TODO:...
//				view->fMouseEventOptions = 0;
				view->MouseUp(where);
			} else
				target->MessageReceived(msg);

			break;
		}

		case B_MOUSE_MOVED:
		{
			if (BView* view = dynamic_cast<BView*>(target)) {
// TODO:...
uint32 transit = B_INSIDE_VIEW;
#if 0
				uint32 eventOptions = view->fEventOptions
					| view->fMouseEventOptions;
				bool noHistory = eventOptions & B_NO_POINTER_HISTORY;
				bool dropIfLate = !(eventOptions & B_FULL_POINTER_HISTORY);

				bigtime_t eventTime;
				if (msg->FindInt64("when", (int64*)&eventTime) < B_OK)
					eventTime = system_time();

				uint32 transit;
				msg->FindInt32("be:transit", (int32*)&transit);
				// don't drop late messages with these important transit values
				if (transit == B_ENTERED_VIEW || transit == B_EXITED_VIEW)
					dropIfLate = false;

				// TODO: The dropping code may have the following problem:
				// On slower computers, 20ms may just be to abitious a delay.
				// There, we might constantly check the message queue for a
				// newer message, not find any, and still use the only but
				// later than 20ms message, which of course makes the whole
				// thing later than need be. An adaptive delay would be
				// kind of neat, but would probably use additional BWindow
				// members to count the successful versus fruitless queue
				// searches and the delay value itself or something similar.

				if (noHistory
					|| (dropIfLate && (system_time() - eventTime > 20000))) {
					// filter out older mouse moved messages in the queue
					_DequeueAll();
					BMessageQueue* queue = MessageQueue();
					queue->Lock();

					BMessage* moved;
					for (int32 i = 0; (moved = queue->FindMessage(i)) != NULL;
							i++) {
						if (moved != msg && moved->what == B_MOUSE_MOVED) {
							// there is a newer mouse moved message in the
							// queue, just ignore the current one, the newer one
							// will be handled here eventually
							queue->Unlock();
							return;
						}
					}
					queue->Unlock();
				}
#endif

				BPoint where;
//				uint32 buttons;
				msg->FindPoint("be:view_where", &where);
//				msg->FindInt32("buttons", (int32*)&buttons);

#if 0
				delete fIdleMouseRunner;

				if (transit != B_EXITED_VIEW && transit != B_OUTSIDE_VIEW) {
					// Start new idle runner
					BMessage idle(B_MOUSE_IDLE);
					idle.AddPoint("be:view_where", where);
					fIdleMouseRunner = new BMessageRunner(
						BMessenger(NULL, this), &idle,
						BToolTipManager::Manager()->ShowDelay(), 1);
				} else {
					fIdleMouseRunner = NULL;
					if (dynamic_cast<BPrivate::ToolTipWindow*>(this) == NULL)
						BToolTipManager::Manager()->HideTip();
				}
#endif

				BMessage* dragMessage = NULL;
				if (msg->HasMessage("be:drag_message")) {
					dragMessage = new BMessage();
					if (msg->FindMessage("be:drag_message", dragMessage)
							!= B_OK) {
						delete dragMessage;
						dragMessage = NULL;
					}
				}

				view->MouseMoved(where, transit, dragMessage);
				delete dragMessage;
			} else
				target->MessageReceived(msg);

			break;
		}

#if 0
		case B_PULSE:
			if (target == this && fPulseRunner) {
				fTopView->_Pulse();
				fLink->Flush();
			} else
				target->MessageReceived(msg);
			break;

		case _UPDATE_:
		{
//bigtime_t now = system_time();
//bigtime_t drawTime = 0;
			STRACE(("info:BWindow handling _UPDATE_.\n"));

			fLink->StartMessage(AS_BEGIN_UPDATE);
			fInTransaction = true;

			int32 code;
			if (fLink->FlushWithReply(code) == B_OK
				&& code == B_OK) {
				// read current window position and size first,
				// the update rect is in screen coordinates...
				// so we need to be up to date
				BPoint origin;
				fLink->Read<BPoint>(&origin);
				float width;
				float height;
				fLink->Read<float>(&width);
				fLink->Read<float>(&height);
				if (origin != fFrame.LeftTop()) {
					// TODO: remove code duplicatation with
					// B_WINDOW_MOVED case...
					//printf("window position was not up to date\n");
					fFrame.OffsetTo(origin);
					FrameMoved(origin);
				}
				if (width != fFrame.Width() || height != fFrame.Height()) {
					// TODO: remove code duplicatation with
					// B_WINDOW_RESIZED case...
					//printf("window size was not up to date\n");
					fFrame.right = fFrame.left + width;
					fFrame.bottom = fFrame.top + height;

					_AdoptResize();
					FrameResized(width, height);
				}

				// read tokens for views that need to be drawn
				// NOTE: we need to read the tokens completely
				// first, we cannot draw views in between reading
				// the tokens, since other communication would likely
				// mess up the data in the link.
				struct ViewUpdateInfo {
					int32 token;
					BRect updateRect;
				};
				BList infos(20);
				while (true) {
					// read next token and create/add ViewUpdateInfo
					int32 token;
					status_t error = fLink->Read<int32>(&token);
					if (error < B_OK || token == B_NULL_TOKEN)
						break;
					ViewUpdateInfo* info = new(std::nothrow) ViewUpdateInfo;
					if (info == NULL || !infos.AddItem(info)) {
						delete info;
						break;
					}
					info->token = token;
					// read culmulated update rect (is in screen coords)
					error = fLink->Read<BRect>(&(info->updateRect));
					if (error < B_OK)
						break;
				}
				// draw
				int32 count = infos.CountItems();
				for (int32 i = 0; i < count; i++) {
//bigtime_t drawStart = system_time();
					ViewUpdateInfo* info
						= (ViewUpdateInfo*)infos.ItemAtFast(i);
					if (BView* view = _FindView(info->token))
						view->_Draw(info->updateRect);
					else {
						printf("_UPDATE_ - didn't find view by token: %ld\n",
							info->token);
					}
//drawTime += system_time() - drawStart;
				}
				// NOTE: The tokens are actually hirachically sorted,
				// so traversing the list in revers and calling
				// child->_DrawAfterChildren() actually works like intended.
				for (int32 i = count - 1; i >= 0; i--) {
					ViewUpdateInfo* info
						= (ViewUpdateInfo*)infos.ItemAtFast(i);
					if (BView* view = _FindView(info->token))
						view->_DrawAfterChildren(info->updateRect);
					delete info;
				}

//printf("  %ld views drawn, total Draw() time: %lld\n", count, drawTime);
			}

			fLink->StartMessage(AS_END_UPDATE);
			fLink->Flush();
			fInTransaction = false;
			fUpdateRequested = false;

//printf("BWindow(%s) - UPDATE took %lld usecs\n", Title(), system_time() - now);
			break;
		}

		case _MENUS_DONE_:
			MenusEnded();
			break;

		// These two are obviously some kind of old scripting messages
		// this is NOT an app_server message and we have to be cautious
		case B_WINDOW_MOVE_BY:
		{
			BPoint offset;
			if (msg->FindPoint("data", &offset) == B_OK)
				MoveBy(offset.x, offset.y);
			else
				msg->SendReply(B_MESSAGE_NOT_UNDERSTOOD);
			break;
		}

		// this is NOT an app_server message and we have to be cautious
		case B_WINDOW_MOVE_TO:
		{
			BPoint origin;
			if (msg->FindPoint("data", &origin) == B_OK)
				MoveTo(origin);
			else
				msg->SendReply(B_MESSAGE_NOT_UNDERSTOOD);
			break;
		}

		case B_LAYOUT_WINDOW:
		{
			Layout(false);
			break;
		}
#endif

		default:
			BLooper::DispatchMessage(msg, target);
			break;
	}
}


void
BWindow::_WidgetAdded(QWidget* widget)
{
	BView* view = dynamic_cast<BView*>(widget);
	if (view != NULL)
		view->_AttachToWindow(this);

	foreach (QObject* child, widget->children()) {
		if (QWidget* childWidget = dynamic_cast<QWidget*>(child))
			_WidgetAdded(childWidget);
	}

	if (view != NULL)
		view->_AllAttachedToWindow();
}


void
BWindow::_WidgetRemoved(QWidget* widget)
{
	BView* view = dynamic_cast<BView*>(widget);
	if (view != NULL)
		view->_DetachFromWindow();

	foreach (QObject* child, widget->children()) {
		if (QWidget* childWidget = dynamic_cast<QWidget*>(child))
			_WidgetRemoved(childWidget);
	}

	if (view != NULL)
		view->_AllDetachedFromWindow();
}
