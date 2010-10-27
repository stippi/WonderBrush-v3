#ifndef VIEW_STATE_H
#define VIEW_STATE_H

#include "StateView.h"

class BMessage;
class Command;

class ViewState {
public:
								ViewState(StateView* view);
								ViewState(const ViewState& other);
	virtual						~ViewState();

	// ViewState interface
	virtual	void				Init();
	virtual	void				Cleanup();

	virtual	void				Draw(BView* into, BRect updateRect);
	virtual	bool				MessageReceived(BMessage* message,
									Command** _command);

	// mouse tracking
	virtual	void				MouseDown(const MouseInfo& info);
	virtual	void				MouseMoved(const MouseInfo& info);
	virtual	Command*			MouseUp();

	// modifiers
	virtual	void				ModifiersChanged(uint32 modifiers);


	// TODO: mouse wheel
	virtual	bool				HandleKeyDown(const StateView::KeyEvent& event,
									Command** _command);
	virtual	bool				HandleKeyUp(const StateView::KeyEvent& event,
									Command** _command);


	virtual	bool				UpdateCursor();
	virtual	BRect				Bounds() const;
			void				UpdateBounds();

	inline	uint32				PressedMouseButtons() const
									{ return fMouseInfo->buttons; }

	inline	bool				IsFirstButtonDown() const
									{ return fMouseInfo->buttons
										& B_PRIMARY_MOUSE_BUTTON; }
	inline	bool				IsSecondButtonDown() const
									{ return fMouseInfo->buttons
										& B_SECONDARY_MOUSE_BUTTON; }
	inline	bool				IsThirdButtonDown() const
									{ return fMouseInfo->buttons
										& B_TERTIARY_MOUSE_BUTTON; }

	inline	BPoint				MousePos() const
									{ return fMouseInfo->position; }

	inline	uint32				Modifiers() const
									{ return fMouseInfo->modifiers; }

			void				TransformCanvasToView(BPoint* point) const;
			void				TransformViewToCanvas(BPoint* point) const;

			void				TransformCanvasToView(BRect* rect) const;
			void				TransformViewToCanvas(BRect* rect) const;

			float				ZoomLevel() const;

			void				Invalidate(BRect canvasBounds);

	inline	StateView*			View() const
									{ return fView; }

protected:
			StateView*			fView;

			// NOTE: the intention of using a pointer
			// to a MouseInfo struct is that all
			// ViewStates belonging to the same StateView
			// should have the same pointer, so that
			// they will all be up to date with the same info
			const MouseInfo*	fMouseInfo;

			BRect				fBounds;
};

#endif // VIEW_STATE_H
