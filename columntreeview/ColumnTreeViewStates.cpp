// ColumnTreeViewStates.cpp

#include <algobase.h>
#include <stdio.h>

#include <InterfaceDefs.h>
#include <Message.h>
#include <OS.h>
#include <Window.h>

#include "ColumnTreeViewStates.h"
#include "Column.h"
#include "ColumnHeader.h"
#include "ColumnTreeItem.h"
#include "ColumnTreeView.h"
#include "Debug.h"

using namespace ColumnTreeViewStates;

//#define ldebug	printf
#define ldebug	nodebug

// State

// constructor
State::State(ColumnTreeView* listView, BPoint point)
	: fListView(listView),
	  fStartPoint(point)
{
}

// destructor
State::~State()
{
}

// Entered
void
State::Entered(BPoint point, const BMessage* message)
{
}

// Exited
void
State::Exited(BPoint point, const BMessage* message)
{
}

// Moved
void
State::Moved(BPoint point, uint32 transit, const BMessage* message)
{
}

// Pressed
void
State::Pressed(BPoint point, uint32 buttons, uint32 modifiers, int32 clicks)
{
}

// Released
void
State::Released(BPoint point, uint32 buttons, uint32 modifiers)
{
}

// GetMouseButtons
void
State::GetMouseButtons(uint32 *buttons, int32* clicks) const
{
	fListView->Window()->CurrentMessage()->FindInt32("buttons",
													   (int32 *)buttons);
	fListView->Window()->CurrentMessage()->FindInt32("clicks", clicks);
}

// When
bigtime_t
State::When()
{
	bigtime_t when;
	if (fListView->Window()->CurrentMessage()->FindInt64("when", &when)
		!= B_OK) {
		when = system_time();
	}
	return when;
}

// IsClick
bool
State::IsClick(bigtime_t first, bigtime_t second)
{
	bigtime_t time = 500000L;
	get_click_speed(&time);
	return (second - first <= time);
}

// IsOverView
bool
State::IsOverView(BPoint point) const
{
	BRect rect(fListView->Parent()->Bounds());
	rect.OffsetBy(fListView->Bounds().LeftTop()
				  - fListView->Frame().LeftTop());
	rect = rect & fListView->Bounds();
	return rect.Contains(point);
}

// ReleaseState
void
State::ReleaseState(BPoint point)
{
	if (IsOverView(point))
		fListView->_ChangeState(new InsideState(fListView, point));
	else
		fListView->_ChangeState(new OutsideState(fListView));
}


// IgnoreState

// constructor
IgnoreState::IgnoreState(ColumnTreeView* listView)
	: State(listView, BPoint())
{
ldebug("IgnoreState::IgnoreState()\n");
}

// destructor
IgnoreState::~IgnoreState()
{
}

// Exited
void
IgnoreState::Exited(BPoint point, const BMessage* message)
{
	fListView->_ChangeState(new OutsideState(fListView));
}


// InsideState

// constructor
InsideState::InsideState(ColumnTreeView* listView, BPoint point)
	: State(listView, point)
{
ldebug("InsideState::InsideState()\n");
}

// destructor
InsideState::~InsideState()
{
}

// Exited
void
InsideState::Exited(BPoint point, const BMessage* message)
{
	fListView->_ChangeState(new OutsideState(fListView));
}

// Pressed
void
InsideState::Pressed(BPoint point, uint32 buttons, uint32 modifiers,
					 int32 clicks)
{
	if (buttons & B_PRIMARY_MOUSE_BUTTON) {
		int32 index = fListView->IndexOf(point);
		bool wasSelected = false;
		bool selectOnRelease = false;
		if (index < 0) {
			// clicked outside of the items rect: deselect all
			fListView->DeselectAll();
		} else {
printf("item frame:        ");
fListView->_ItemFrame(index).PrintToStream();
printf("item handle frame: ");
fListView->_ItemHandleFrame(index).PrintToStream();
printf("column 0 frame:    ");
fListView->_VisibleColumnFrame(0L).PrintToStream();
			if (fListView->_ItemHandleFrame(index).Contains(point)) {
				// hit the item handle: toggle expanded state
				if (ColumnTreeItem* item = fListView->ItemAt(index)) {
					if (item->IsExpanded())
						fListView->CollapseItem(item);
					else
						fListView->ExpandItem(item);
					// do not enter pressed state -- bail out here
					return;
				}
			}
			// hit an item: select/deselect according to modifiers
			wasSelected = fListView->IsItemSelected(index);
			int32 firstSelected = fListView->CurrentSelection(0);
			int32 lastSelected =
				fListView->CurrentSelection(
					fListView->fSelectedItems.CountItems() - 1);
			bool selectionEmpty = (firstSelected < 0);
			// shift
			if (modifiers & B_SHIFT_KEY && !selectionEmpty
				&& fListView->SelectionMode() == CLV_MULTIPLE_SELECTION) {
				firstSelected = min(firstSelected, index);
				lastSelected = max(lastSelected, index);
				fListView->Select(firstSelected, lastSelected, true);
			// option
			} else if (modifiers & B_OPTION_KEY) {
				if (wasSelected)
					fListView->Deselect(index);
				else
					fListView->Select(index, true);
			// no modifier
			} else {
				if (wasSelected)
					selectOnRelease = true;
				else
					fListView->Select(index, false);
			}
		}
		// switch to pressed state
		fListView->_ChangeState(new PressedState(fListView, point, index,
								wasSelected, selectOnRelease, clicks, When()));
	}
}


// OutsideState

// constructor
OutsideState::OutsideState(ColumnTreeView* listView)
	: State(listView, BPoint())
{
ldebug("OutsideState::OutsideState()\n");
}

// destructor
OutsideState::~OutsideState()
{
}

// Entered
void
OutsideState::Entered(BPoint point, const BMessage* message)
{
	uint32 buttons = 0;
	int32 clicks = 1;
	GetMouseButtons(&buttons, &clicks);
//	if ((buttons & (B_PRIMARY_MOUSE_BUTTON | B_SECONDARY_MOUSE_BUTTON |
//					B_TERTIARY_MOUSE_BUTTON)) ||
//		message) {
//		fListView->_ChangeState(new IgnoreState(fListView));
//	} else
		fListView->_ChangeState(new InsideState(fListView, point));
}


// PressedState

// constructor
PressedState::PressedState(ColumnTreeView* listView, BPoint point,
						   int32 index, bool wasSelected, bool selectOnRelease,
						   int32 clicks, bigtime_t clickTime)
	: State(listView, point),
	  fItemIndex(index),
	  fWasSelected(wasSelected),
	  fSelectOnRelease(selectOnRelease),
	  fClicks(clicks),
	  fClickTime(clickTime)
{
ldebug("PressedState::PressedState()\n");
	fListView->SetMouseEventMask(B_POINTER_EVENTS);
}

// destructor
PressedState::~PressedState()
{
}

// Moved
void
PressedState::Moved(BPoint point, uint32 transit, const BMessage* message)
{
	// initiate dragging
	if (fItemIndex >= 0 &&
		fabs(fStartPoint.x - point.x) + fabs(fStartPoint.y - point.y) > 2.0) {
		// initiate dragging
		fListView->InitiateDrag(point, fItemIndex, fWasSelected);
		ReleaseState(point);
	}
}

// Released
void
PressedState::Released(BPoint point, uint32 buttons, uint32 modifiers)
{
	if (fSelectOnRelease)
		fListView->Select(fItemIndex, false);
	// check if double clicked
	if (fClicks == 2 && IsClick(fClickTime, When())) {
		// invoke
		fListView->_InternalInvoke(fListView->fInvocationMessage);
	}
	ReleaseState(point);
}


