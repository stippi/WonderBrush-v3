// LabelColumnHeader.cpp

#include <stdio.h>

#include <View.h>

#include "LabelColumnHeader.h"
#include "ColumnTreeViewColors.h"

// constructor
LabelColumnHeader::LabelColumnHeader(const char* label)
	: ColumnHeader(),
	  fLabel(label)
{
}

// destructor
LabelColumnHeader::~LabelColumnHeader()
{
}

static pattern kDottedLinePattern = {
	{ 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa }
};

// Draw
void
LabelColumnHeader::Draw(BView* view, BRect frame, BRect updateRect,
						uint32 flags, const column_header_colors* colors)
{
	bool pressed = (flags & COLUMN_PRESSED);
	bool primarySortKey = (flags & COLUMN_PRIMARY_SORT_KEY);
	bool secondarySortKey = (flags & COLUMN_SECONDARY_SORT_KEY);
	DrawBackground(view, frame, updateRect, flags, colors);
	DrawFrame(view, frame, flags, colors);
	if (pressed)
		view->SetHighColor(colors->pressed_foreground);
	else
		view->SetHighColor(colors->foreground);
	// draw the title text
	// probably we should...
	// limit the clipping region to the interior of the box
	font_height fontHeight;
	view->GetFontHeight(&fontHeight);
	BRect textRect(frame.InsetByCopy(1.0, 1.0));
	BPoint textPoint(textRect.left + 8.0, textRect.bottom - 2.0 - fontHeight.descent);

	BString label(fLabel);
	view->TruncateString(&label, B_TRUNCATE_END, textRect.right - textPoint.x);
	float textWidth = view->StringWidth(label.String());

	view->SetDrawingMode(B_OP_OVER);
	view->DrawString(label.String(), textPoint);
	textPoint.y += 1.0f;
	if (primarySortKey) {
		view->StrokeLine(textPoint, BPoint(textPoint.x + textWidth,
						 textPoint.y));
	} else if (secondarySortKey) {
		view->StrokeLine(textPoint, BPoint(textPoint.x + textWidth,
						 textPoint.y), kDottedLinePattern);
	}
	view->SetDrawingMode(B_OP_COPY);
}

// SetLabel
void
LabelColumnHeader::SetLabel(const char* label)
{
	fLabel.SetTo(label);
	Invalidate();
}

// Label
const char*
LabelColumnHeader::Label() const
{
	return fLabel.String();
}

