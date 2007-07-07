// ColumnHeader.cpp

#include <View.h>

#include "ColumnHeader.h"
#include "ColumnTreeViewColors.h"

// constructor
ColumnHeader::ColumnHeader()
	: fParentColumn(NULL),
	  fXOffset(0),
	  fWidth(0),
	  fFirstHeader(0),
	  fLastHeader(0),
	  fVisibleHeader(0)
{
}

// destructor
ColumnHeader::~ColumnHeader()
{
}

// Draw
void
ColumnHeader::Draw(BView* view, BRect frame, BRect updateRect, uint32 flags,
				   const column_header_colors* colors)
{
}

// DrawBackground
void
ColumnHeader::DrawBackground(BView* view, BRect frame, BRect rect,
							 uint32 flags, const column_header_colors* colors)
{
	bool pressed = (flags & COLUMN_PRESSED);
	rect = rect & frame.InsetBySelf(1.0, 1.0);
	// set the colors according to the selection state
	if (pressed)
		view->SetLowColor(colors->pressed_background);
	else
		view->SetLowColor(colors->background);
	view->FillRect(rect, B_SOLID_LOW);
}

// DrawFrame
void
ColumnHeader::DrawFrame(BView* view, BRect frame, uint32 flags,
						const column_header_colors* colors)
{
	bool pressed = (flags & COLUMN_PRESSED);
	bool resizing = (flags & COLUMN_RESIZING);
	float left = frame.left;
	float right = frame.right;
	float top = frame.top;
	float bottom = frame.bottom;
	// set the colors according to the selection state
	rgb_color Highlight, Shadow;
	if (pressed) {
		Highlight = colors->pressed_highlight;
		Shadow = colors->pressed_shadow;
		view->SetLowColor(colors->pressed_background);
		view->SetHighColor(colors->pressed_foreground);
	} else {
		Highlight = colors->highlight;
		Shadow = colors->shadow;
		view->SetLowColor(colors->background);
		view->SetHighColor(colors->foreground);
	}
	// draw the frame
	view->BeginLineArray(4);
	view->AddLine(BPoint(left, top), BPoint(right, top), Highlight); //top line
	view->AddLine(BPoint(left, bottom), BPoint(left, top - 1.0), Highlight); //left line
	view->AddLine(BPoint(left + 1.0, bottom), BPoint(right, bottom), colors->light_shadow); //bottom line
	if (resizing)
		view->AddLine(BPoint(right, bottom), BPoint(right, top),
			ui_color(B_KEYBOARD_NAVIGATION_COLOR)); //right line
	else
		view->AddLine(BPoint(right, bottom), BPoint(right, top), colors->shadow); //right line
	view->EndLineArray();
}

// Invalidate
void
ColumnHeader::Invalidate()
{
	if (Column* parent = ParentColumn())
		parent->InvalidateHeader();
}

// SetParentColumn
void
ColumnHeader::SetParentColumn(Column* column)
{
	fParentColumn = column;
}

// ParentColumn
Column*
ColumnHeader::ParentColumn() const
{
	return fParentColumn;
}

// SetXOffset
void
ColumnHeader::SetXOffset(float offset)
{
	fXOffset = offset;
}

// XOffset
float
ColumnHeader::XOffset() const
{
	return fXOffset;
}

// SetWidth
void
ColumnHeader::SetWidth(float width)
{
	fWidth = width;
}

// Width
float
ColumnHeader::Width() const
{
	return fWidth;
}

// SetHeaderRange
void
ColumnHeader::SetHeaderRange(int32 first, int32 last)
{
	fFirstHeader = first;
	fLastHeader = last;
}

// SetFirstHeader
void
ColumnHeader::SetFirstHeader(int32 first)
{
	fFirstHeader = first;
}

// SetLastHeader
void
ColumnHeader::SetLastHeader(int32 last)
{
	fLastHeader = last;
}

// FirstHeader
int32
ColumnHeader::FirstHeader() const
{
	return fFirstHeader;
}

// LastHeader
int32
ColumnHeader::LastHeader() const
{
	return fLastHeader;
}

// RangeSize
int32
ColumnHeader::RangeSize() const
{
	return fLastHeader - fFirstHeader + 1;
}

// SetVisibleHeader
void
ColumnHeader::SetVisibleHeader(int32 index)
{
	fVisibleHeader = index;
}

// VisibleHeader
int32
ColumnHeader::VisibleHeader() const
{
	return fVisibleHeader;
}

