// ColumnTreeViewStates.h

#ifndef COLUMN_TREE_VIEW_STATES_H
#define COLUMN_TREE_VIEW_STATES_H

#include <Point.h>

class BMessage;

class ColumnTreeView;

namespace ColumnTreeViewStates {

// State
class State {
 public:
								State(ColumnTreeView* listView,
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
			ColumnTreeView*		fListView;
			BPoint				fStartPoint;
};

// IgnoreState
class IgnoreState : public State {
 public:
								IgnoreState(ColumnTreeView* listView);
	virtual						~IgnoreState();

	virtual	void				Exited(BPoint point, const BMessage* message);
};

// InsideState
class InsideState : public State {
 public:
								InsideState(ColumnTreeView* listView,
											BPoint point);
	virtual						~InsideState();

	virtual	void				Exited(BPoint point, const BMessage* message);
	virtual	void				Pressed(BPoint point, uint32 buttons,
										uint32 modifiers, int32 clicks);

 private:
};

// OutsideState
class OutsideState : public State {
 public:
								OutsideState(ColumnTreeView* listView);
	virtual						~OutsideState();

	virtual	void				Entered(BPoint point, const BMessage* message);
};

// PressedState
class PressedState : public State {
 public:
								PressedState(ColumnTreeView* listView,
											 BPoint point, int32 index,
											 bool wasSelected,
											 bool selectOnRelease,
											 int32 clicks,
											 bigtime_t clickTime);
	virtual						~PressedState();

	virtual	void				Moved(BPoint point, uint32 transit,
									  const BMessage* message);
	virtual	void				Released(BPoint point, uint32 buttons,
										 uint32 modifiers);

 private:
			int32				fItemIndex;
			bool				fWasSelected;
			bool				fSelectOnRelease;
			int32				fClicks;
			bigtime_t			fClickTime;
};



}	// namespace ColumnTreeViewStates


#endif	// COLUMN_TREE_VIEW_STATES_H
