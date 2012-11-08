#ifndef STATE_VIEW_H
#define STATE_VIEW_H

#include <Message.h>
#include <View.h>

#include "PlatformViewMixin.h"

class BMessageFilter;
class Command;
class CommandStack;
class RWLocker;
class ViewState;

struct MouseInfo {
								MouseInfo();
								MouseInfo(const MouseInfo& other);

			MouseInfo&			operator=(const MouseInfo& other);

			uint32				buttons;
			BPoint				position;
			BPoint				tilt;
			float				pressure;
			uint32				transit;
			uint32				clicks;
			uint32				modifiers;
			BMessage			dragMessage;
};

class StateView : public PlatformViewMixin<BView> {
public:
	class KeyEvent;

								StateView(BRect frame, const char* name,
									uint32 resizingMode, uint32 flags);
								StateView(const char* name, uint32 flags);
	virtual						~StateView();

	// BView interface
	virtual	void				AttachedToWindow();
	virtual	void				DetachedFromWindow();
	virtual void				PlatformDraw(PlatformDrawContext& drawContext);
	virtual	void				MessageReceived(BMessage* message);

	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 transit,
										   const BMessage* dragMessage);
	virtual	void				MouseUp(BPoint where);

	virtual	void				KeyDown(const char* bytes, int32 numBytes);
	virtual	void				KeyUp(const char* bytes, int32 numBytes);

	// StateView interface
			void				SetState(ViewState* state);
			void				UpdateStateCursor();
			BRect				ViewStateBounds();
	virtual	void				ViewStateBoundsChanged();

			void				Draw(PlatformDrawContext& drawContext);

	virtual	bool				MouseWheelChanged(BPoint where,
									float x, float y);
	virtual	void				NothingClicked(BPoint where,
									uint32 buttons, uint32 clicks);

			bool				HandleKeyDown(const KeyEvent& event,
									BHandler* originalTarget);
			bool				HandleKeyUp(const KeyEvent& event,
									BHandler* originalTarget);

			const ::MouseInfo*	LastMouseInfo() const
									{ return &fLastMouseInfo; }
			const ::MouseInfo*	MouseInfo() const
									{ return &fMouseInfo; }

	virtual	void				ConvertFromCanvas(BPoint* point) const;
	virtual	void				ConvertToCanvas(BPoint* point) const;

	virtual	void				ConvertFromCanvas(BRect* rect) const;
	virtual	void				ConvertToCanvas(BRect* rect) const;

	virtual	float				ZoomLevel() const;

	virtual	void				InvalidateCanvas(const BRect& bounds);

	virtual	void				FilterMouse(BPoint* where) const;

	virtual	ViewState*			StateForDragMessage(const BMessage* message);

			void				SetLocker(RWLocker* locker);
			RWLocker*			Locker() const
									{ return fLocker; }

			void				SetCommandStack(::CommandStack* stack);
			::CommandStack*		CommandStack() const
									{ return fCommandStack; }

			void				SetUpdateTarget(BHandler* target,
												uint32 command);

			void				SetCatchAllEvents(bool catchAll);

			status_t			PerformCommand(Command* command);

			void				TriggerUpdate();

protected:
	virtual	bool				_HandleKeyDown(const KeyEvent& event,
									BHandler* originalTarget);
	virtual	bool				_HandleKeyUp(const KeyEvent& event,
									BHandler* originalTarget);

			void				_InstallEventFilter();
			void				_RemoveEventFilter();

			void				_ExtractTabletInfo(const BMessage* message,
									::MouseInfo& info);

protected:
			ViewState*			fCurrentState;
			ViewState*			fDropAnticipatingState;
				// the drop anticipation state is some
				// kind of "temporary" state that is
				// used on top of the current state (it
				// doesn't replace it)
			::MouseInfo			fMouseInfo;
			::MouseInfo			fLastMouseInfo;

			::CommandStack*		fCommandStack;
			RWLocker*			fLocker;

			BMessageFilter*		fEventFilter;
			bool				fCatchAllEvents;

			BHandler*			fUpdateTarget;
			uint32				fUpdateCommand;
};

class StateView::KeyEvent {
public:
								KeyEvent(uint32 key, const char* bytes,
										 size_t length, uint32 modifiers)
									: key(key),
									  bytes(bytes),
									  length(length),
									  modifiers(modifiers)
								{ }

			uint32				key;
			const char*			bytes;
			size_t				length;
			uint32				modifiers;
};

#endif // STATE_VIEW_H
