// ColumnHeaderViewStates.h

#ifndef COLUMN_HEADER_VIEW_STATES_H
#define COLUMN_HEADER_VIEW_STATES_H

#include <Point.h>

class BMessage;

class ColumnHeaderView;

namespace ColumnHeaderViewStates {

// State
class State {
 public:
								State(ColumnHeaderView* headerView,
									  BPoint point);
	virtual						~State();

	virtual	void				Entered(BPoint point, const BMessage* message);
	virtual	void				Exited(BPoint point, const BMessage* message);
	virtual	void				Moved(BPoint point, uint32 transit,
									  const BMessage* message);
	virtual	void				Pressed(BPoint point, uint32 buttons,
										uint32 modifiers, int32 clicks);
	virtual	void				Released(BPoint point, uint32 buttons,
										 uint32 modifiers);

			void				GetMouseButtons(uint32 *buttons,
												int32* clicks) const;

			bigtime_t			When();
			bool				IsClick(bigtime_t first, bigtime_t second);

			bool				IsOverView(BPoint point) const;
			
			void				ReleaseState(BPoint point);

 protected:
			ColumnHeaderView*	fHeaderView;
			BPoint				fStartPoint;
};

// DraggingState
class DraggingState : public State {
 public:
								DraggingState(ColumnHeaderView* headerView,
											  BPoint point, int32 index);
	virtual						~DraggingState();

	virtual	void				Moved(BPoint point, uint32 transit,
									  const BMessage* message);
	virtual	void				Released(BPoint point, uint32 buttons,
										 uint32 modifiers);

 private:
			int32				fHeaderIndex;
};

// IgnoreState
class IgnoreState : public State {
 public:
								IgnoreState(ColumnHeaderView* headerView);
	virtual						~IgnoreState();

	virtual	void				Exited(BPoint point, const BMessage* message);
};

// InsideState
class InsideState : public State {
 public:
								InsideState(ColumnHeaderView* headerView,
											BPoint point);
	virtual						~InsideState();

	virtual	void				Exited(BPoint point, const BMessage* message);
	virtual	void				Moved(BPoint point, uint32 transit,
									  const BMessage* message);
	virtual	void				Pressed(BPoint point, uint32 buttons,
										uint32 modifiers, int32 clicks);

 private:
	enum inside_state {
		INSIDE_RESIZABLE,
		INSIDE_DRAGABLE,
		INSIDE_NEUTRAL,
	};

			inside_state		fInsideState;
			int32				fHeaderIndex;

			void				_UpdateInsideState(BPoint point);

			inside_state		_InsideStateFor(BPoint point,
												int32* headerIndex);
};

// OutsideState
class OutsideState : public State {
 public:
								OutsideState(ColumnHeaderView* headerView);
	virtual						~OutsideState();

	virtual	void				Entered(BPoint point, const BMessage* message);
};

// PressedState
class PressedState : public State {
 public:
								PressedState(ColumnHeaderView* headerView,
											 BPoint point, int32 index,
											 bigtime_t clickTime);
	virtual						~PressedState();

	virtual	void				Moved(BPoint point, uint32 transit,
									  const BMessage* message);
	virtual	void				Released(BPoint point, uint32 buttons,
										 uint32 modifiers);

 private:
			int32				fHeaderIndex;
			bigtime_t			fClickTime;
};

// ResizingState
class ResizingState : public State {
 public:
								ResizingState(ColumnHeaderView* headerView,
											  BPoint point, int32 index);
	virtual						~ResizingState();

	virtual	void				Moved(BPoint point, uint32 transit,
									  const BMessage* message);
	virtual	void				Released(BPoint point, uint32 buttons,
										 uint32 modifiers);

 private:
			int32				fHeaderIndex;
			float				fColumnWidth;
};



}	// namespace ColumnHeaderViewStates


#endif	// COLUMN_HEADER_VIEW_STATES_H
