#include "StateView.h"

#include <new>

#include <string.h>

#include <Message.h>
#include <MessageFilter.h>
#include <Messenger.h>
#include <Screen.h>
#include <Window.h>

#include "EditManager.h"
#include "RWLocker.h"
#include "ViewState.h"


MouseInfo::MouseInfo()
	:
	buttons(0),
	position(-10000, -10000),
	tilt(B_ORIGIN),
	pressure(1.0f),
	transit(B_OUTSIDE_VIEW),
	clicks(0),
	modifiers(::modifiers())
{
}

MouseInfo::MouseInfo(const MouseInfo& other)
{
	*this = other;
}

MouseInfo&
MouseInfo::operator=(const MouseInfo& other)
{
	buttons = other.buttons;
	position = other.position;
	tilt = other.tilt;
	pressure = other.pressure;
	transit = other.transit;
	clicks = other.clicks;
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
	PlatformViewMixin<BView>(frame, name, resizingMode, flags),
	fCurrentState(NULL),
	fDropAnticipatingState(NULL),

	fMouseInfo(),
	fLastMouseInfo(),

	fEditManager(NULL),
	fLocker(NULL),

	fEventFilter(NULL),
	fCatchAllEvents(false),

	fUpdateTarget(NULL),
	fUpdateCommand(0)
{
}


// constructor
StateView::StateView(const char* name, uint32 flags)
	:
	PlatformViewMixin<BView>(name, flags),
	fCurrentState(NULL),
	fDropAnticipatingState(NULL),

	fMouseInfo(),
	fLastMouseInfo(),

	fEditManager(NULL),
	fLocker(NULL),

	fEventFilter(NULL),
	fCatchAllEvents(false),

	fUpdateTarget(NULL),
	fUpdateCommand(0)
{
}


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
StateView::PlatformDraw(PlatformDrawContext& drawContext)
{
	Draw(drawContext);
}

// MessageReceived
void
StateView::MessageReceived(BMessage* message)
{
	// let the state handle the message if it wants
	if (fCurrentState != NULL) {
		AutoWriteLocker locker(fLocker);
		if (fLocker && !locker.IsLocked())
			return;

		UndoableEdit* edit = NULL;
		if (fCurrentState->MessageReceived(message, &edit)) {
			PerformEdit(edit);
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
			fMouseInfo.modifiers = mods;

			ViewState* state = fDropAnticipatingState != NULL ?
				fDropAnticipatingState : fCurrentState;
			if (state != NULL)
				state->ModifiersChanged(mods);

			// call MouseMoved() of drop anticipation state
			// in case something needs to change because of
			// different modifiers
			if (fDropAnticipatingState != NULL)
				fDropAnticipatingState->MouseMoved(fMouseInfo);
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
	if (fLocker != NULL && !fLocker->WriteLock())
		return;

	::MouseInfo info(fMouseInfo);
	info.buttons = B_PRIMARY_MOUSE_BUTTON;
	info.clicks = 1;
	info.position = where;

	// query more info from the windows current message if available
	BMessage* message = Window() ? Window()->CurrentMessage() : NULL;
	if (message != NULL) {
		message->FindInt32("buttons", (int32*)&info.buttons);
		message->FindInt32("clicks", (int32*)&info.clicks);
		_ExtractTabletInfo(message, info);
	}

	if (fCurrentState != NULL)
		fCurrentState->MouseDown(info);

	// update mouse info *after* having called the ViewState hook
	fMouseInfo = info;

	if (fLocker != NULL)
		fLocker->WriteUnlock();
}

// MouseMoved
void
StateView::MouseMoved(BPoint where, uint32 transit,
	const BMessage* dragMessage)
{
	if (fLocker != NULL && !fLocker->WriteLock())
		return;

	if (dragMessage != NULL && fDropAnticipatingState == NULL) {
		// switch to a drop anticipating state if there is one available
		fDropAnticipatingState = StateForDragMessage(dragMessage);
		if (fDropAnticipatingState != NULL)
			fDropAnticipatingState->Init();
	}

	// TODO: I don't like this too much
	if ((dragMessage == NULL || transit == B_EXITED_VIEW)
		&& fDropAnticipatingState != NULL) {
		fDropAnticipatingState->Cleanup();
		fDropAnticipatingState = NULL;
	}

	fLastMouseInfo = fMouseInfo;

	// update mouse info
	fMouseInfo.position = where;
	fMouseInfo.transit = transit;

	// query more info from the windows current message if available
	BMessage* message = Window() ? Window()->CurrentMessage() : NULL;
	if (message != NULL) {
		message->FindInt32("buttons", (int32*)&fMouseInfo.buttons);
		_ExtractTabletInfo(message, fMouseInfo);
	}

	// cache drag message
	if (dragMessage != NULL)
		fMouseInfo.dragMessage = *dragMessage;
	else
		fMouseInfo.dragMessage.what = 0;

	if (fDropAnticipatingState != NULL)
		fDropAnticipatingState->MouseMoved(fMouseInfo);
	else if (fCurrentState != NULL) {
		fCurrentState->MouseMoved(fMouseInfo);
		if (fMouseInfo.buttons != 0)
			TriggerUpdate();
	}

	if (fLocker != NULL)
		fLocker->WriteUnlock();
}

// MouseUp
void
StateView::MouseUp(BPoint where)
{
	if (fLocker != NULL && !fLocker->WriteLock())
		return;

	if (fDropAnticipatingState != NULL) {
		PerformEdit(fDropAnticipatingState->MouseUp());
		fDropAnticipatingState->Cleanup();
		fDropAnticipatingState = NULL;

		if (fCurrentState != NULL)
			fCurrentState->MouseMoved(fMouseInfo);
	} else {
		if (fCurrentState != NULL) {
			PerformEdit(fCurrentState->MouseUp());
			TriggerUpdate();
		}
	}

	// update mouse info *after* having called the ViewState hook
	fMouseInfo.buttons = 0;

	if (fLocker != NULL)
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

// InvalidateCanvas
void
StateView::InvalidateCanvas(const BRect& bounds)
{
	Invalidate(bounds);
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
	if (fCurrentState != NULL)
		fCurrentState->Cleanup();

	fCurrentState = state;

	if (fCurrentState != NULL)
		fCurrentState->Init();
}

// UpdateStateCursor
void
StateView::UpdateStateCursor()
{
	if (fCurrentState == NULL || !fCurrentState->UpdateCursor())
		SetViewCursor(B_CURSOR_SYSTEM_DEFAULT, true);
}

// ViewStateBounds
BRect
StateView::ViewStateBounds()
{
	if (fCurrentState != NULL)
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
StateView::Draw(PlatformDrawContext& drawContext)
{
	if (fLocker != NULL && !fLocker->ReadLock()) {
		return;
	}

	if (fCurrentState != NULL)
		fCurrentState->Draw(drawContext);

	if (fDropAnticipatingState != NULL)
		fDropAnticipatingState->Draw(drawContext);

	if (fLocker != NULL)
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
	if (fLocker != NULL && !locker.IsLocked())
		return false;

	if (fCurrentState != NULL) {
		UndoableEdit* edit = NULL;
		if (fCurrentState->HandleKeyDown(event, &edit)) {
			PerformEdit(edit);
			return true;
		}
	}
	return _HandleKeyDown(event, originalTarget);
}

// HandleKeyUp
bool
StateView::HandleKeyUp(const KeyEvent& event, BHandler* originalTarget)
{
	AutoWriteLocker locker(fLocker);
	if (fLocker != NULL && !locker.IsLocked())
		return false;

	if (fCurrentState != NULL) {
		UndoableEdit* edit = NULL;
		if (fCurrentState->HandleKeyUp(event, &edit)) {
			PerformEdit(edit);
			return true;
		}
	}

	return _HandleKeyUp(event, originalTarget);
}

// StateForDragMessage
ViewState*
StateView::StateForDragMessage(const BMessage* message)
{
	return NULL;
}

// SetEditManager
void
StateView::SetEditManager(::EditManager* stack)
{
	fEditManager = stack;
}

// SetLocker
void
StateView::SetLocker(RWLocker* locker)
{
	fLocker = locker;
}

// SetUpdateTarget
void
StateView::SetUpdateTarget(BHandler* target, uint32 edit)
{
	fUpdateTarget = target;
	fUpdateCommand = edit;
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

// PerformEdit
status_t
StateView::PerformEdit(UndoableEdit* edit)
{
	if (edit != NULL)
		return PerformEdit(UndoableEditRef(edit, true));

	return B_BAD_VALUE;
}

// PerformEdit
status_t
StateView::PerformEdit(const UndoableEditRef& edit)
{
	if (fEditManager != NULL)
		return fEditManager->Perform(edit);

	return B_NO_INIT;
}

// PostMessage
status_t
StateView::PostMessage(uint32 what)
{
	BMessage message(what);
	return PostMessage(message);
}

// PostMessage
status_t
StateView::PostMessage(BMessage& message)
{
	BMessenger messenger(this);
	return messenger.SendMessage(&message);
}

// TriggerUpdate
void
StateView::TriggerUpdate()
{
	if (fUpdateTarget != NULL)
		BMessenger(fUpdateTarget).SendMessage(fUpdateCommand);
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

	if (fEventFilter == NULL)
		fEventFilter = new(std::nothrow) EventFilter(this);

	if (fEventFilter == NULL || Window() == NULL)
		return;

	Window()->AddCommonFilter(fEventFilter);
}

// _RemoveEventFilter
void
StateView::_RemoveEventFilter()
{
	if (fEventFilter == NULL || Window() == NULL)
		return;

	Window()->RemoveCommonFilter(fEventFilter);
}

// _ExtractTabletInfo
void
StateView::_ExtractTabletInfo(const BMessage* message, ::MouseInfo& info)
{
	if (message->FindFloat("be:tablet_pressure", &info.pressure) != B_OK)
		info.pressure = 1.0f;
	message->FindFloat("be:tablet_tilt_x", &info.tilt.x);
	message->FindFloat("be:tablet_tilt_y", &info.tilt.y);
	float x;
	float y;
	if (message->FindFloat("be:tablet_x", &x) == B_OK
		&& message->FindFloat("be:tablet_y", &y) == B_OK) {
		BScreen screen(Window());
		if (screen.IsValid()) {
			info.position.x = x * screen.Frame().Width();
			info.position.y = y * screen.Frame().Height();
			ConvertFromScreen(&info.position);
		}
	}
}

