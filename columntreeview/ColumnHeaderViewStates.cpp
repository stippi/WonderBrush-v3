// ColumnHeaderViewStates.cpp

#include <algobase.h>
#include <stdio.h>

#include <InterfaceDefs.h>
#include <Message.h>
#include <OS.h>
#include <Window.h>

#include "ColumnHeaderViewStates.h"
#include "Column.h"
#include "ColumnHeader.h"
#include "ColumnHeaderView.h"
#include "ColumnTreeView.h"
#include "Debug.h"

using namespace ColumnHeaderViewStates;

//#define ldebug	printf
#define ldebug	nodebug

// State

// constructor
State::State(ColumnHeaderView* headerView, BPoint point)
	: fHeaderView(headerView),
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
	fHeaderView->Window()->CurrentMessage()->FindInt32("buttons",
													   (int32 *)buttons);
	fHeaderView->Window()->CurrentMessage()->FindInt32("clicks", clicks);
}

// When
bigtime_t
State::When()
{
	bigtime_t when;
	if (fHeaderView->Window()->CurrentMessage()->FindInt64("when", &when)
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
	BRect rect(fHeaderView->Parent()->Bounds());
	rect.OffsetBy(fHeaderView->Bounds().LeftTop()
				  - fHeaderView->Frame().LeftTop());
	rect = rect & fHeaderView->Bounds();
	return rect.Contains(point);
}

// ReleaseState
void
State::ReleaseState(BPoint point)
{
	if (IsOverView(point))
		fHeaderView->_ChangeState(new InsideState(fHeaderView, point));
	else
		fHeaderView->_ChangeState(new OutsideState(fHeaderView));
}


// DraggingState

// constructor
DraggingState::DraggingState(ColumnHeaderView* headerView, BPoint point,
							 int32 index)
	: State(headerView, point),
	  fHeaderIndex(index)
{
ldebug("DraggingState::DraggingState()\n");
	ColumnHeader* header = fHeaderView->HeaderAt(fHeaderIndex);
	header->ParentColumn()->AddFlags(COLUMN_MOVING);
}

// destructor
DraggingState::~DraggingState()
{
}

// Moved
void
DraggingState::Moved(BPoint point, uint32 transit, const BMessage* message)
{
	// Find out which header lies next to the current mouse position.
	int32 dest = fHeaderView->IndexOf(point.x);
	if (dest < 0) {
		if (point.x < fHeaderView->_HeadersRect().left)
			dest = fHeaderView->_VisibleHeaderIndexFor(0);
		else {
			dest = fHeaderView->_VisibleHeaderIndexFor(
						fHeaderView->CountHeaders() - 1);
		}
	}
	// If the destination is different, we take a deeper look.
	if (dest != fHeaderIndex) {
		// Adjust dest according to niceness.
		BRect destRect(fHeaderView->_HeaderRect(dest));
		float headerWidth = fHeaderView->_HeaderRect(fHeaderIndex).Width();
		if (dest < fHeaderIndex) {
			if (destRect.left + headerWidth < point.x) {
				dest = fHeaderView->_VisibleHeaderIndexFor(
					fHeaderView->HeaderAt(dest)->LastHeader() + 1);
			}
		} else {
			if (destRect.right - headerWidth > point.x) {
				dest = fHeaderView->_VisibleHeaderIndexFor(
					fHeaderView->HeaderAt(dest)->FirstHeader() - 1);
			}
		}
		// Destination found -- move the columns.
		if (dest != fHeaderIndex) {
			ColumnHeader* header = fHeaderView->HeaderAt(fHeaderIndex);
			ColumnHeader* destHeader = fHeaderView->HeaderAt(dest);
			if (dest < fHeaderIndex) {
				// move before dest
				fHeaderView->ParentView()->MoveVisibleColumns(
						header->FirstHeader(), destHeader->FirstHeader(),
						header->RangeSize());
			} else {
				// move behind dest
				fHeaderView->ParentView()->MoveVisibleColumns(
						header->FirstHeader(),
						destHeader->LastHeader() - header->RangeSize() + 1,
						header->RangeSize());
			}
			fHeaderIndex = fHeaderView->IndexOf(header);
		}
	}
}

// Released
void
DraggingState::Released(BPoint point, uint32 buttons, uint32 modifiers)
{
	ColumnHeader* header = fHeaderView->HeaderAt(fHeaderIndex);
	header->ParentColumn()->ClearFlags(COLUMN_PRESSED | COLUMN_MOVING);
	fHeaderView->_InvalidateHeaders(fHeaderIndex, 1);
	ReleaseState(point);
}


// IgnoreState

// constructor
IgnoreState::IgnoreState(ColumnHeaderView* headerView)
	: State(headerView, BPoint())
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
	fHeaderView->_ChangeState(new OutsideState(fHeaderView));
}


// InsideState

// constructor
InsideState::InsideState(ColumnHeaderView* headerView, BPoint point)
	: State(headerView, point),
	  fInsideState(INSIDE_NEUTRAL),
	  fHeaderIndex(-1)
{
ldebug("InsideState::InsideState()\n");
	// work around a BeOS bug:
	fHeaderView->_SetStandardCursor();
	// initialize the inside state
	_UpdateInsideState(point);
}

// destructor
InsideState::~InsideState()
{
}

// Exited
void
InsideState::Exited(BPoint point, const BMessage* message)
{
	if (fInsideState == INSIDE_RESIZABLE)
		fHeaderView->_SetStandardCursor();
	fHeaderView->_ChangeState(new OutsideState(fHeaderView));
}

// Moved
void
InsideState::Moved(BPoint point, uint32 transit, const BMessage* message)
{
	_UpdateInsideState(point);
}

// Pressed
void
InsideState::Pressed(BPoint point, uint32 buttons, uint32 modifiers,
					 int32 clicks)
{
	if (buttons & B_PRIMARY_MOUSE_BUTTON
		|| buttons & B_SECONDARY_MOUSE_BUTTON
		|| buttons & B_TERTIARY_MOUSE_BUTTON) {
		switch (fInsideState) {
			case INSIDE_RESIZABLE:
				fHeaderView->_ChangeState(new ResizingState(fHeaderView, point,
															fHeaderIndex));
				break;
			case INSIDE_DRAGABLE:
				fHeaderView->_ChangeState(new PressedState(fHeaderView, point,
														   fHeaderIndex,
														   When()));
				break;
			default:
				break;
		}
	}
}

// _UpdateInsideState
void
InsideState::_UpdateInsideState(BPoint point)
{
	int32 index = -1;
	inside_state state = _InsideStateFor(point, &index);
	if (fInsideState != state) {
		if (state == INSIDE_RESIZABLE)
			fHeaderView->_SetHorizontalResizeCursor();
		else if (fInsideState == INSIDE_RESIZABLE)
			fHeaderView->_SetStandardCursor();
		fInsideState = state;
		fHeaderIndex = index;
		// ...
ldebug("inside state: %x, index: %ld\n", fInsideState, fHeaderIndex);
	} else if (index != fHeaderIndex) {
		fHeaderIndex = index;
		// ...
ldebug("inside state: %x, index: %ld\n", fInsideState, fHeaderIndex);
	}
}

// _InsideStateFor
InsideState::inside_state
InsideState::_InsideStateFor(BPoint point, int32* headerIndex)
{
	inside_state state = INSIDE_NEUTRAL;
	const float MAX_DIST = 2.0;
	if (fHeaderView->Bounds().Contains(point)) {
		// check resizable
		if (fHeaderView->CountHeaders() > 0) {
			float posx = point.x - MAX_DIST - 1.0;
			int32 index = fHeaderView->IndexOf(BPoint(posx, point.y));
			if (index >= 0) {
				if (fHeaderView->_HeaderRect(index).right - posx <=
					MAX_DIST * 2) {
					*headerIndex = index;
					state = INSIDE_RESIZABLE;
				}
			}
		}
		// check dragable
		if (state == INSIDE_NEUTRAL) {
			int32 index = fHeaderView->IndexOf(point);
			if (index >= 0) {
				*headerIndex = index;
				state = INSIDE_DRAGABLE;
			}
		}
	}
	return state;
}


// OutsideState

// constructor
OutsideState::OutsideState(ColumnHeaderView* headerView)
	: State(headerView, BPoint())
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
	if ((buttons & (B_PRIMARY_MOUSE_BUTTON | B_SECONDARY_MOUSE_BUTTON |
					B_TERTIARY_MOUSE_BUTTON)) ||
		message) {
		fHeaderView->_ChangeState(new IgnoreState(fHeaderView));
	} else
		fHeaderView->_ChangeState(new InsideState(fHeaderView, point));
}


// PressedState

// constructor
PressedState::PressedState(ColumnHeaderView* headerView, BPoint point,
						   int32 index, bigtime_t clickTime)
	: State(headerView, point),
	  fHeaderIndex(index),
	  fClickTime(clickTime)
{
ldebug("PressedState::PressedState()\n");
	fHeaderView->SetMouseEventMask(B_POINTER_EVENTS);
	ColumnHeader* header = fHeaderView->HeaderAt(fHeaderIndex);
	header->ParentColumn()->AddFlags(COLUMN_PRESSED);
	fHeaderView->_InvalidateHeaders(fHeaderIndex, 1);
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
	if (fHeaderView->HeaderAt(fHeaderIndex)->ParentColumn()->IsMovable() &&
		fabs(fStartPoint.x - point.x) + fabs(fStartPoint.y - point.y) > 2.0) {
		fHeaderView->_ChangeState(new DraggingState(fHeaderView, fStartPoint,
													 fHeaderIndex));
	}
}

// Released
void
PressedState::Released(BPoint point, uint32 buttons, uint32 modifiers)
{
	// check if clicked and column is sort keyable
	Column* column = fHeaderView->HeaderAt(fHeaderIndex)->ParentColumn();
	if (IsClick(fClickTime, When()) && column->IsSortKeyable()) {
		bool inverse = (column->Flags() & COLUMN_SORT_INVERSE);
		if (modifiers & B_SHIFT_KEY) {
			if (column->Flags() & COLUMN_SECONDARY_SORT_KEY) {
				// already secondary sort key
				if (inverse) {
					fHeaderView->ParentView()->SetSecondarySortVisibleColumn(
							-1, false);
				} else {
					fHeaderView->ParentView()->SetSecondarySortVisibleColumn(
							fHeaderIndex, true);
				}
			} else {
				fHeaderView->ParentView()->SetSecondarySortVisibleColumn(
						fHeaderIndex, false);
			}
		} else {
			if (column->Flags() & COLUMN_PRIMARY_SORT_KEY) {
				// already primary sort key
				if (inverse) {
					fHeaderView->ParentView()->SetPrimarySortVisibleColumn(
							-1, false);
				} else {
					fHeaderView->ParentView()->SetPrimarySortVisibleColumn(
							fHeaderIndex, true);
				}
			} else {
				fHeaderView->ParentView()->SetPrimarySortVisibleColumn(
						fHeaderIndex, false);
			}
		}
	}
	// leave pressed state
	ColumnHeader* header = fHeaderView->HeaderAt(fHeaderIndex);
	header->ParentColumn()->ClearFlags(COLUMN_PRESSED);
	fHeaderView->_InvalidateHeaders(fHeaderIndex, 1);
	ReleaseState(point);
}


// ResizingState

// constructor
ResizingState::ResizingState(ColumnHeaderView* headerView, BPoint point,
							 int32 index)
	: State(headerView, point),
	  fHeaderIndex(index),
	  fColumnWidth(0)
{
ldebug("ResizingState::ResizingState()\n");
	ColumnHeader* header = fHeaderView->HeaderAt(fHeaderIndex);
	fColumnWidth = header->ColumnWidth();
	fHeaderView->SetMouseEventMask(B_POINTER_EVENTS);
	header->ParentColumn()->AddFlags(COLUMN_RESIZING);
	fHeaderView->_InvalidateHeaders(fHeaderIndex, 1);
	fHeaderView->ParentView()->DisableScrolling();
}

// destructor
ResizingState::~ResizingState()
{
}

// Moved
void
ResizingState::Moved(BPoint point, uint32 transit, const BMessage* message)
{
	ColumnHeader* header = fHeaderView->HeaderAt(fHeaderIndex);
	float width = fColumnWidth + point.x - fStartPoint.x;
	// We just try it... if the width is out of the width limits, the
	// column catches the problem.
	if (width != header->ColumnWidth())
		fHeaderView->ParentView()->ResizeVisibleColumn(fHeaderIndex, width);
}

// Released
void
ResizingState::Released(BPoint point, uint32 buttons, uint32 modifiers)
{
	ColumnHeader* header = fHeaderView->HeaderAt(fHeaderIndex);
	header->ParentColumn()->ClearFlags(COLUMN_RESIZING);
	fHeaderView->_InvalidateHeaders(fHeaderIndex, 1);
	fHeaderView->_SetStandardCursor();
	fHeaderView->ParentView()->EnableScrolling();
	ReleaseState(point);
}



