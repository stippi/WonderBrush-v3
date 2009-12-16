#include "StateView.h"

#include <new>

#include <Message.h>
#include <MessageFilter.h>
#include <Window.h>

#include "Command.h"
#include "CommandStack.h"
#include "RWLocker.h"
#include "ViewState.h"

using std::nothrow;


mouse_info&
mouse_info::operator=(const mouse_info& other)
{
	buttons = other.buttons;
	position = other.position;
	transit = other.transit;
	modifiers = other.modifiers;
	dragMessage = other.dragMessage;

	return *this;
}


class EventFilter : public BMessageFilter {
 public:
	EventFilter(StateView* target)
		: BMessageFilter(B_ANY_DELIVERY, B_ANY_SOURCE),
		  fTarget(target)
		{
		}
	virtual	~EventFilter()
		{
		}
	virtual	filter_result	Filter(BMessage* message, BHandler** target)
		{
			filter_result result = B_DISPATCH_MESSAGE;
			switch (message->what) {
				case B_KEY_DOWN: {
					uint32 key;
					const char* bytes;
					uint32 modifiers;
					if (message->FindInt32("raw_char", (int32*)&key) >= B_OK
						&& message->FindInt32("modifiers",
											  (int32*)&modifiers) >= B_OK
						&& message->FindString("bytes", &bytes) == B_OK)
						if (fTarget->HandleKeyDown(
								StateView::KeyEvent(key, bytes, strlen(bytes),
									modifiers), *target))
							result = B_SKIP_MESSAGE;
					break;
				}
				case B_KEY_UP: {
					uint32 key;
					const char* bytes;
					uint32 modifiers;
					if (message->FindInt32("raw_char", (int32*)&key) >= B_OK
						&& message->FindInt32("modifiers",
											  (int32*)&modifiers) >= B_OK
						&& message->FindString("bytes", &bytes) == B_OK)
						if (fTarget->HandleKeyUp(
								StateView::KeyEvent(key, bytes, strlen(bytes),
									modifiers), *target))
							result = B_SKIP_MESSAGE;
					break;

				}
				case B_MODIFIERS_CHANGED:
					*target = fTarget;
					break;

				case B_MOUSE_WHEEL_CHANGED: {
					float x;
					float y;
					if (message->FindFloat("be:wheel_delta_x", &x) >= B_OK
						&& message->FindFloat("be:wheel_delta_y", &y) >= B_OK) {
						if (fTarget->MouseWheelChanged(
							fTarget->MouseInfo()->position, x, y))
							result = B_SKIP_MESSAGE;
					}
					break;
				}
				default:
					break;
			}
			return result;
		}
 private:
 	StateView*		fTarget;
};

// #pragma mark -

// constructor
StateView::StateView(BRect frame, const char* name, uint32 resizingMode,
		uint32 flags)
	:
	BView(frame, name, resizingMode, flags),
	fCurrentState(NULL),
	fDropAnticipatingState(NULL),

	fMouseInfo(),
	fLastMouseInfo(),

	fCommandStack(NULL),
	fLocker(NULL),

	fEventFilter(NULL),
	fCatchAllEvents(false),

	fUpdateTarget(NULL),
	fUpdateCommand(0)
{
}

#ifdef __HAIKU__

// constructor
StateView::StateView(const char* name, uint32 flags)
	:
	BView(name, flags),
	fCurrentState(NULL),
	fDropAnticipatingState(NULL),

	fMouseInfo(),
	fLastMouseInfo(),

	fCommandStack(NULL),
	fLocker(NULL),

	fEventFilter(NULL),
	fCatchAllEvents(false),

	fUpdateTarget(NULL),
	fUpdateCommand(0)
{
}

#endif // __HAIKU__

// destructor
StateView::~StateView()
{
	delete fEventFilter;
}

// #pragma mark -

// AttachedToWindow
void
StateView::AttachedToWindow()
{
	_InstallEventFilter();

	BView::AttachedToWindow();
}

// DetachedFromWindow
void
StateView::DetachedFromWindow()
{
	_RemoveEventFilter();

	BView::DetachedFromWindow();
}

// Draw
void
StateView::Draw(BRect updateRect)
{
	Draw(this, updateRect);
}

// MessageReceived
void
StateView::MessageReceived(BMessage* message)
{
	// let the state handle the message if it wants
	if (fCurrentState) {
		AutoWriteLocker locker(fLocker);
		if (fLocker && !locker.IsLocked())
			return;

		Command* command = NULL;
		if (fCurrentState->MessageReceived(message, &command)) {
			PerformCommand(command);
			return;
		}
	}

	switch (message->what) {
		case B_MODIFIERS_CHANGED: {
			// NOTE: only received if view has focus or
			// SetCatchAllEvents() is set to true
			if (fDropAnticipatingState) {
				// switch to a new drop anticipating state
				// if necessary
				ViewState* state = StateForDragMessage(
					&fMouseInfo.dragMessage);
				if (state != fDropAnticipatingState) {
					fDropAnticipatingState->Cleanup();
					fDropAnticipatingState = state;
					if (fDropAnticipatingState)
						fDropAnticipatingState->Init();
				}
			}
			uint32 mods;
			if (message->FindInt32("modifiers", (int32*)&mods) != B_OK)
				mods = modifiers();
			ViewState* state = fDropAnticipatingState ?
				fDropAnticipatingState : fCurrentState;
			if (state)
				state->ModifiersChanged(mods);

			fMouseInfo.modifiers = mods;

			// call MouseMoved() of drop anticipation state
			// in case something needs to change because of
			// different modifiers
			if (fDropAnticipatingState) {
				fDropAnticipatingState->MouseMoved(
					fMouseInfo.position, fMouseInfo.transit,
					&fMouseInfo.dragMessage);
			}
			break;
		}
		case B_MOUSE_WHEEL_CHANGED: {
			float xDelta, yDelta;
			if (message->FindFloat("be:wheel_delta_x", &xDelta) < B_OK)
				xDelta = 0.0;
			if (message->FindFloat("be:wheel_delta_y", &yDelta) < B_OK)
				yDelta = 0.0;
			if (xDelta != 0.0 || yDelta != 0.0)
				MouseWheelChanged(MouseInfo()->position, xDelta, yDelta);
			break;
		}
		default:
			BView::MessageReceived(message);
	}
}

// #pragma mark -

// MouseDown
void
StateView::MouseDown(BPoint where)
{
	if (fLocker && !fLocker->WriteLock())
		return;

	// query more info from the windows current message if available
	uint32 buttons;
	uint32 clicks;
	BMessage* message = Window() ? Window()->CurrentMessage() : NULL;
	if (!message || message->FindInt32("buttons", (int32*)&buttons) != B_OK)
		buttons = B_PRIMARY_MOUSE_BUTTON;
	if (!message || message->FindInt32("clicks", (int32*)&clicks) != B_OK)
		clicks = 1;

	if (fCurrentState)
		fCurrentState->MouseDown(where, buttons, clicks);

	// update mouse info *after* having called the ViewState hook
	fMouseInfo.buttons = buttons;
	fMouseInfo.position = where;

	if (fLocker)
		fLocker->WriteUnlock();
}

// MouseMoved
void
StateView::MouseMoved(BPoint where, uint32 transit,
					  const BMessage* dragMessage)
{
	if (fLocker && !fLocker->WriteLock())
		return;

	if (dragMessage && !fDropAnticipatingState) {
		// switch to a drop anticipating state if there is one available
		fDropAnticipatingState = StateForDragMessage(dragMessage);
		if (fDropAnticipatingState)
			fDropAnticipatingState->Init();
	}

	// TODO: I don't like this too much
	if ((!dragMessage || transit == B_EXITED_VIEW)
		&& fDropAnticipatingState) {
		fDropAnticipatingState->Cleanup();
		fDropAnticipatingState = NULL;
	}

	fLastMouseInfo = fMouseInfo;

	// update mouse info
	fMouseInfo.position = where;
	fMouseInfo.transit = transit;
	// cache drag message
	if (dragMessage)
		fMouseInfo.dragMessage = *dragMessage;
	else
		fMouseInfo.dragMessage.what = 0;


	if (fDropAnticipatingState)
		fDropAnticipatingState->MouseMoved(where, transit, dragMessage);
	else {
		if (fCurrentState) {
			fCurrentState->MouseMoved(where, transit, dragMessage);
			if (fMouseInfo.buttons != 0)
				TriggerUpdate();
		}
	}


	if (fLocker)
		fLocker->WriteUnlock();
}

// MouseUp
void
StateView::MouseUp(BPoint where)
{
	if (fLocker && !fLocker->WriteLock())
		return;

	if (fDropAnticipatingState) {
		PerformCommand(fDropAnticipatingState->MouseUp());
		fDropAnticipatingState->Cleanup();
		fDropAnticipatingState = NULL;

		if (fCurrentState) {
			fCurrentState->MouseMoved(fMouseInfo.position, fMouseInfo.transit,
									  NULL);
		}
	} else {
		if (fCurrentState) {
			PerformCommand(fCurrentState->MouseUp());
			TriggerUpdate();
		}
	}

	// update mouse info *after* having called the ViewState hook
	fMouseInfo.buttons = 0;

	if (fLocker)
		fLocker->WriteUnlock();
}

// #pragma mark -

// KeyDown
void
StateView::KeyDown(const char* bytes, int32 numBytes)
{
	uint32 key;
	uint32 modifiers;
	BMessage* message = Window() ? Window()->CurrentMessage() : NULL;
	if (message
		&& message->FindInt32("raw_char", (int32*)&key) >= B_OK
		&& message->FindInt32("modifiers", (int32*)&modifiers) >= B_OK) {
		if (HandleKeyDown(KeyEvent(key, bytes, numBytes, modifiers), this))
			return;
	}
	BView::KeyDown(bytes, numBytes);
}

// KeyUp
void
StateView::KeyUp(const char* bytes, int32 numBytes)
{
	uint32 key;
	uint32 modifiers;
	BMessage* message = Window() ? Window()->CurrentMessage() : NULL;
	if (message
		&& message->FindInt32("raw_char", (int32*)&key) >= B_OK
		&& message->FindInt32("modifiers", (int32*)&modifiers) >= B_OK) {
		if (HandleKeyUp(KeyEvent(key, bytes, numBytes, modifiers), this))
			return;
	}
	BView::KeyUp(bytes, numBytes);
}

// #pragma mark -

// ConvertFromCanvas
void
StateView::ConvertFromCanvas(BPoint* point) const
{
}

// ConvertToCanvas
void
StateView::ConvertToCanvas(BPoint* point) const
{
}

// ConvertFromCanvas
void
StateView::ConvertFromCanvas(BRect* rect) const
{
}

// ConvertToCanvas
void
StateView::ConvertToCanvas(BRect* rect) const
{
}

// ZoomLevel
float
StateView::ZoomLevel() const
{
	return 1.0f;
}

// FilterMouse
void
StateView::FilterMouse(BPoint* point) const
{
}

// #pragma mark -

// SetState
void
StateView::SetState(ViewState* state)
{
	if (fCurrentState == state)
		return;

	// switch states as appropriate
	if (fCurrentState)
		fCurrentState->Cleanup();

	fCurrentState = state;

	if (fCurrentState)
		fCurrentState->Init();
}

// UpdateStateCursor
void
StateView::UpdateStateCursor()
{
	if (!fCurrentState || !fCurrentState->UpdateCursor()) {
		SetViewCursor(B_CURSOR_SYSTEM_DEFAULT, true);
	}
}

// ViewStateBounds
BRect
StateView::ViewStateBounds()
{
	if (fCurrentState)
		return fCurrentState->Bounds();
	return BRect(0, 0, -1, -1);
}

// ViewStateBoundsChanged
void
StateView::ViewStateBoundsChanged()
{
}

// Draw
void
StateView::Draw(BView* into, BRect updateRect)
{
	if (fLocker && !fLocker->ReadLock()) {
		return;
	}

	if (fCurrentState)
		fCurrentState->Draw(into, updateRect);

	if (fDropAnticipatingState)
		fDropAnticipatingState->Draw(into, updateRect);

	if (fLocker)
		fLocker->ReadUnlock();
}

// MouseWheelChanged
bool
StateView::MouseWheelChanged(BPoint where, float x, float y)
{
	return false;
}

// NothingClicked
void
StateView::NothingClicked(BPoint where, uint32 buttons, uint32 clicks)
{
}

// HandleKeyDown
bool
StateView::HandleKeyDown(const KeyEvent& event, BHandler* originalTarget)
{
	AutoWriteLocker locker(fLocker);
	if (fLocker && !locker.IsLocked())
		return false;

	if (_HandleKeyDown(event, originalTarget))
		return true;

	if (fCurrentState) {
		Command* command = NULL;
		if (fCurrentState->HandleKeyDown(event, &command)) {
			PerformCommand(command);
			return true;
		}
	}
	return false;
}

// HandleKeyUp
bool
StateView::HandleKeyUp(const KeyEvent& event, BHandler* originalTarget)
{
	AutoWriteLocker locker(fLocker);
	if (fLocker && !locker.IsLocked())
		return false;

	if (_HandleKeyUp(event, originalTarget))
		return true;

	if (fCurrentState) {
		Command* command = NULL;
		if (fCurrentState->HandleKeyUp(event, &command)) {
			PerformCommand(command);
			return true;
		}
	}
	return false;
}

// StateForDragMessage
ViewState*
StateView::StateForDragMessage(const BMessage* message)
{
	return NULL;
}

// SetCommandStack
void
StateView::SetCommandStack(::CommandStack* stack)
{
	fCommandStack = stack;
}

// SetLocker
void
StateView::SetLocker(RWLocker* locker)
{
	fLocker = locker;
}

// SetUpdateTarget
void
StateView::SetUpdateTarget(BHandler* target, uint32 command)
{
	fUpdateTarget = target;
	fUpdateCommand = command;
}

// SetCatchAllEvents
void
StateView::SetCatchAllEvents(bool catchAll)
{
	if (fCatchAllEvents == catchAll)
		return;

	fCatchAllEvents = catchAll;

	if (fCatchAllEvents)
		_InstallEventFilter();
	else
		_RemoveEventFilter();
}

// PerformCommand
status_t
StateView::PerformCommand(Command* command)
{
	if (fCommandStack)
		return fCommandStack->Perform(command);

	// if there is no command stack, then nobody
	// else feels responsible...
	delete command;

	return B_NO_INIT;
}

// TriggerUpdate
void
StateView::TriggerUpdate()
{
	if (fUpdateTarget && fUpdateTarget->Looper()) {
		fUpdateTarget->Looper()->PostMessage(fUpdateCommand);
	}
}

// #pragma mark -

// _HandleKeyDown
bool
StateView::_HandleKeyDown(const KeyEvent& event, BHandler* originalTarget)
{
	return false;
}

// _HandleKeyUp
bool
StateView::_HandleKeyUp(const KeyEvent& event, BHandler* originalTarget)
{
	return false;
}

// _InstallEventFilter
void
StateView::_InstallEventFilter()
{
	if (!fCatchAllEvents)
		return;

	if (!fEventFilter)
		fEventFilter = new (nothrow) EventFilter(this);

	if (!fEventFilter || !Window())
		return;

	Window()->AddCommonFilter(fEventFilter);
}

// _RemoveEventFilter
void
StateView::_RemoveEventFilter()
{
	if (!fEventFilter || !Window())
		return;

	Window()->RemoveCommonFilter(fEventFilter);
}

