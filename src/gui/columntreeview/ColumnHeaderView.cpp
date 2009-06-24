// ColumnHeaderView.cpp

//#include <algobase.h>
#include <stdio.h>
#include <math.h>

#include <Cursor.h>
#include <Font.h>
#include <Message.h>
#include <Window.h>

#include "ColumnHeaderView.h"
#include "ColumnHeader.h"
#include "ColumnHeaderViewStates.h"
#include "ColumnTreeViewColors.h"

// Tracker like
/*const unsigned char kHorizontalResizeCursor[] = {
	16,1,7,7,
	0,0,1,0,1,0,1,0,9,32,25,48,57,56,121,60,
	57,56,25,48,9,32,1,0,1,0,1,0,0,0,0,0,
	3,128,3,128,3,128,15,224,31,240,63,248,127,252,255,254,
	127,252,63,248,31,240,15,224,3,128,3,128,3,128,0,0
};*/

// as used elsewhere in eXposer
const unsigned char kHorizontalResizeCursor[] = { 16,1,7,7,
	0x0,0x0,0x01,0x80,0x01,0x80,0x01,0x80,
	0x01,0x80,0x11,0x88,0x31,0x8C,0x7D,0xBE,
	0x7D,0xBE,0x31,0x8C,0x11,0x88,0x01,0x80,
	0x01,0x80,0x01,0x80,0x01,0x80,0x0,0x0,

	0x03,0xC0,0x03,0xC0,0x03,0xC0,0x03,0xC0,
	0x3B,0xDC,0x7B,0xDE,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0x7B,0xDE,0x3B,0xDC,
	0x03,0xC0,0x03,0xC0,0x03,0xC0,0x03,0xC0
};


using namespace ColumnHeaderViewStates;

// constructor
ColumnHeaderView::ColumnHeaderView()
	: BView(BRect(0.0, 0.0, 10.0, 10.0), NULL, B_FOLLOW_NONE, B_WILL_DRAW),
	  fParentView(NULL),
	  fHeaders(10),
	  fVisibleHeaders(10),
	  fHeight(ceilf(be_plain_font->Size() * 1.5)),
	  fHorizontalResizeCursor(NULL),
	  fState(new OutsideState(this)),
	  fColors(new column_header_view_colors(kDefaultColumnHeaderViewColors))
{
	SetViewColor(B_TRANSPARENT_32_BIT);
//	SetMouseEventMask(B_POINTER_EVENTS);
}

// destructor
ColumnHeaderView::~ColumnHeaderView()
{
	_ChangeState(NULL);
	delete fColors;
	delete fHorizontalResizeCursor;
}

// Draw
void
ColumnHeaderView::Draw(BRect updateRect)
{
	BRect headersRect(_HeadersRect());
	// let the headers draw themselves
	BRect headersUpdateRect(updateRect & headersRect);
	if (headersUpdateRect.IsValid()) {
		// some headers have to be updated
		int32 first = IndexOf(headersUpdateRect.left);
		int32 last = IndexOf(headersUpdateRect.right);
		for (int32 i = first; i <= last; i++) {
			ColumnHeader* header = HeaderAt(i);
			if (header->RangeSize() > 0) {
				BRect headerRect(_HeaderRect(header));
				header->Draw(this, headerRect, headerRect & updateRect,
							 header->ParentColumn()->Flags(),
							 &fColors->header_colors);
			}
		}
	}
	// draw the background
	if (updateRect.right > headersRect.right) {
		updateRect.left = headersRect.right + 1.0f;
		if (updateRect.IsValid()) {
			SetHighColor(fColors->background);
			FillRect(updateRect);
			BeginLineArray(3);
			AddLine(BPoint(updateRect.left, headersRect.bottom),
					BPoint(updateRect.left, headersRect.top),
					fColors->header_colors.highlight);
			AddLine(BPoint(updateRect.left + 1.0, headersRect.top),
					BPoint(updateRect.right, headersRect.top),
					fColors->header_colors.highlight);
			AddLine(BPoint(updateRect.left + 1.0, headersRect.bottom),
					BPoint(updateRect.right, headersRect.bottom),
					fColors->header_colors.light_shadow);
			EndLineArray();
		}
	}
	// shadow at bottom
	SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_3_TINT));
	StrokeLine(Bounds().LeftBottom(), Bounds().RightBottom());
}

// MouseDown
void
ColumnHeaderView::MouseDown(BPoint point)
{
	uint32 buttons = 0;
	uint32 modifiers = 0;
	int32 clicks = 1;
	Window()->CurrentMessage()->FindInt32("buttons", (int32 *)&buttons);
	Window()->CurrentMessage()->FindInt32("modifiers", (int32 *)&modifiers);
	Window()->CurrentMessage()->FindInt32("clicks", &clicks);
	fState->Pressed(point, buttons, modifiers, clicks);
}

// MouseMoved
void
ColumnHeaderView::MouseMoved(BPoint point, uint32 transit,
							 const BMessage* message)
{
	switch (transit) {
		case B_ENTERED_VIEW:
			fState->Entered(point, message);
			break;
		case B_EXITED_VIEW:
			fState->Exited(point, message);
			break;
		default:
			fState->Moved(point, transit, message);
			break;
	}
}

// MouseUp
void
ColumnHeaderView::MouseUp(BPoint point)
{
	uint32 buttons = 0;
	uint32 modifiers = 0;
	Window()->CurrentMessage()->FindInt32("buttons", (int32 *)&buttons);
	Window()->CurrentMessage()->FindInt32("modifiers", (int32 *)&modifiers);
	fState->Released(point, buttons, modifiers);
}

// SetParentView
void
ColumnHeaderView::SetParentView(ColumnTreeView* parent)
{
	fParentView = parent;
}

// ParentView
ColumnTreeView*
ColumnHeaderView::ParentView() const
{
	return fParentView;
}

// Height
float
ColumnHeaderView::Height() const
{
	return fHeight;
}

// AddHeader
void
ColumnHeaderView::AddHeader(ColumnHeader* header, int32 index)
{
	if (header) {
		// check index
		int32 count = CountHeaders();
		if (index < 0 || index > count)
			index = count;
		// add header and invalidate the concerned area
		fHeaders.AddItem((void*)header, index);
		_RebuildVisibleHeaders();
		_InvalidateHeaders(index, -1);
	}
}

// MoveHeader
void
ColumnHeaderView::MoveHeader(int32 oldIndex, int32 newIndex)
{
	if (oldIndex != newIndex) {
		ColumnHeader* header = HeaderAt(oldIndex);
		fHeaders.RemoveItem(oldIndex);
		fHeaders.AddItem((void*)header, newIndex);
		// Update the graphics stuff.
		int32 first = MIN(oldIndex, newIndex);
		int32 last = MAX(oldIndex, newIndex);
		_RebuildVisibleHeaders();
		_InvalidateHeaders(first, last - first + 1);
	}
}

// MoveHeaders
void
ColumnHeaderView::MoveHeaders(int32 index, int32 dest, int32 count)
{
	if (index != dest) {
		BList headers;
		for (int32 i = index; i < index + count; i++)
			headers.AddItem((void*)HeaderAt(i));
		fHeaders.RemoveItems(index, count);
		fHeaders.AddList(&headers, dest);
		// Update the graphics stuff.
		int32 first = MIN(index, dest);
		int32 last = MAX(index, dest) + count - 1;
		_RebuildVisibleHeaders();
		_InvalidateHeaders(first, last - first + 1);
	}
}

// RemoveHeader
ColumnHeader*
ColumnHeaderView::RemoveHeader(int32 index)
{
	ColumnHeader* header = NULL;
	// check index
	if (index >= 0 && index < CountHeaders()) {
		header = (ColumnHeader*)fHeaders.RemoveItem(index);
		_RebuildVisibleHeaders();
		_InvalidateHeaders(index, -1);
	}
	return header;
}

// RemoveHeader
bool
ColumnHeaderView::RemoveHeader(ColumnHeader* header)
{
	bool success = RemoveHeader(IndexOf(header));
	if (success)
		_RebuildVisibleHeaders();
	return success;
}

// ResizeHeader
void
ColumnHeaderView::ResizeHeader(int32 index, float width)
{
	_RebuildVisibleHeaders();
	_InvalidateHeaders(index, -1);
}

// HeaderAt
ColumnHeader*
ColumnHeaderView::HeaderAt(int32 index) const
{
	return (ColumnHeader*)fHeaders.ItemAt(index);
}

// HeaderAt
ColumnHeader*
ColumnHeaderView::HeaderAt(BPoint point) const
{
	return HeaderAt(IndexOf(point));
}

// IndexOf
int32
ColumnHeaderView::IndexOf(ColumnHeader* header) const
{
	return fHeaders.IndexOf((void*)header);
}

// IndexOf
int32
ColumnHeaderView::IndexOf(BPoint point) const
{
	if (_HeadersRect().Contains(point))
		return IndexOf(point.x);
	return -1;
}

// IndexOf
int32
ColumnHeaderView::IndexOf(float x) const
{
	if (x >= 0) {
		for (int32 i = 0; ColumnHeader* header = _VisibleHeaderAt(i); i++) {
			if (x <= header->XOffset() + header->Width())
				return IndexOf(header);
		}
	}
	return -1;
}

// CountHeaders
int32
ColumnHeaderView::CountHeaders() const
{
	return fHeaders.CountItems();
}

// SetColors
void
ColumnHeaderView::SetColors(const column_header_view_colors* colors)
{
	*fColors = *colors;
	Invalidate();
}

// Colors
const column_header_view_colors*
ColumnHeaderView::Colors() const
{
	return fColors;
}

// _VisibleHeaderAt
ColumnHeader*
ColumnHeaderView::_VisibleHeaderAt(int32 i) const
{
	return (ColumnHeader*)fVisibleHeaders.ItemAt(i);
}

// _CountVisibleHeaders
int32
ColumnHeaderView::_CountVisibleHeaders() const
{
	return fVisibleHeaders.CountItems();
}

// _VisibleHeaderIndexFor
int32
ColumnHeaderView::_VisibleHeaderIndexFor(int32 index) const
{
	if (index >=0 && index < CountHeaders())
		return HeaderAt(index)->VisibleHeader();
	return -1;
}

// _VisibleHeaderFor
ColumnHeader*
ColumnHeaderView::_VisibleHeaderFor(int32 index) const
{
	return HeaderAt(_VisibleHeaderIndexFor(index));
}

// _VisibleIndexOf
int32
ColumnHeaderView::_VisibleIndexOf(ColumnHeader* header) const
{
	return fVisibleHeaders.IndexOf((void*)header);
}


// _InvalidateHeaders
//
// Invalidates the region covered by the /count/ headers starting at /index/.
// If /count/ < 0, everything right of the header specified by /index/ is
// invalidated. If /index/ < 0 or >= CountHeaders() the region right of
// the last header is invalidated.
void
ColumnHeaderView::_InvalidateHeaders(int32 index, int32 count)
{
	// nothing to do
	if (count == 0)
		return;
	int32 headerCount = CountHeaders();
	BRect rect(Bounds());
	if (index < 0 || index >= headerCount) {
		// invalidate the region on right of the last header
		if (ColumnHeader* header = _VisibleHeaderFor(headerCount - 1))
			rect.left = header->XOffset() + header->Width() + 1.0f;
	} else {
		if (ColumnHeader* header = _VisibleHeaderFor(index))
			rect.left = header->XOffset();
		if (count > 0) {
			if (index + count > headerCount)
				count = headerCount - index;
			if (ColumnHeader* header = _VisibleHeaderFor(index + count - 1))
				rect.right = header->XOffset() + header->Width();
		}
	}
	Invalidate(rect);
}

// _HeaderRect
//
// Valid for visible headers only.
BRect
ColumnHeaderView::_HeaderRect(ColumnHeader* header) const
{
	BRect rect(_HeadersRect());
	rect.left += header->XOffset();
	rect.right = rect.left + header->Width();
	return rect;
}

// _HeaderRect
//
// Valid for visible headers only.
BRect
ColumnHeaderView::_HeaderRect(int32 index) const
{
	return _HeaderRect(HeaderAt(index));
}

// _HeadersRect
//
// The rect covered by the headers. Invalid if there are no headers.
BRect
ColumnHeaderView::_HeadersRect() const
{
	BRect rect(0.0, 0.0, -1.0, Bounds().Height() - 1.0);
	int32 count = CountHeaders();
	if (count > 0) {
		ColumnHeader* header = _VisibleHeaderFor(count - 1);
		rect.right = header->XOffset() + header->Width();
	}
	return rect;
}

// _RebuildVisibleHeaders
void
ColumnHeaderView::_RebuildVisibleHeaders()
{
	fVisibleHeaders.MakeEmpty();
	int32 count = CountHeaders();
	BList pendingHeaders;
	float pendingWidth = 0;
	float offset = 0;
	ColumnHeader* lastHeader = NULL;
	for (int32 i = 0; ColumnHeader* header = HeaderAt(i); i++) {
		if ((header->Flags() & COLUMN_JOIN_RIGHT) && i < count - 1) {
			// a `join right' header, but not the last of the headers
			header->SetHeaderRange(i, i - 1);
			pendingHeaders.AddItem((void*)header);
			pendingWidth += header->ColumnWidth() + 1.0f;
			lastHeader = NULL;
		} else if ((header->Flags() & COLUMN_JOIN_LEFT) && lastHeader) {
			// a `join left' header, but there was a header to join before
			header->SetHeaderRange(i, i - 1);
			lastHeader->SetWidth(lastHeader->Width() +
								 header->ColumnWidth() + 1.0f);
			offset += header->ColumnWidth() + 1.0f;
			lastHeader->SetLastHeader(i);
			header->SetVisibleHeader(lastHeader->VisibleHeader());
		} else {
			// a standard header
			lastHeader = header;
			header->SetXOffset(offset);
			header->SetWidth(header->ColumnWidth() + pendingWidth);
			offset += header->Width() + 1.0f;
			header->SetHeaderRange(i - pendingHeaders.CountItems(), i);
			header->SetVisibleHeader(i);
			pendingWidth = 0;
			// set the pending headers' visible header
			for (int32 j = 0;
				 ColumnHeader* h = (ColumnHeader*)pendingHeaders.ItemAt(j);
				 j++) {
				h->SetVisibleHeader(i);
			}
			pendingHeaders.MakeEmpty();
			fVisibleHeaders.AddItem((void*)header);
		}
	}
}

// _ChangeState
void
ColumnHeaderView::_ChangeState(State* state)
{
	delete fState;
	fState = state;
}

// _SetHorizontalResizeCursor
void
ColumnHeaderView::_SetHorizontalResizeCursor()
{
	if (!fHorizontalResizeCursor)
		fHorizontalResizeCursor = new BCursor(kHorizontalResizeCursor);
	SetViewCursor(fHorizontalResizeCursor);
}

// _SetStandardCursor
void
ColumnHeaderView::_SetStandardCursor()
{
	SetViewCursor(B_CURSOR_SYSTEM_DEFAULT);
}

