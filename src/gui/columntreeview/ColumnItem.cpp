// ColumnItem.cpp

#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <String.h>
#include <View.h>

#include "ColumnItem.h"
#include "Column.h"
#include "ColumnTreeItem.h"
#include "ColumnTreeViewColors.h"


// ColumnItem

// constructor
ColumnItem::ColumnItem(bool clearBackground)
	: fClearBackground(clearBackground)
{
}

// destructor
ColumnItem::~ColumnItem()
{
}

// Draw
void				
ColumnItem::Draw(BView* view, Column* column, BRect frame, BRect updateRect,
				 uint32 flags, const column_tree_item_colors* colors)
{
}


// BitmapColumnItem

// constructor
BitmapColumnItem::BitmapColumnItem(const BBitmap* bitmap)
	: ColumnItem(true),
	  fBitmap(NULL)
{
	// copy bitmap
	if (bitmap && bitmap->IsValid()) {
		fBitmap = new BBitmap(bitmap->Bounds(), bitmap->ColorSpace());
		if (fBitmap->IsValid())
			memcpy(fBitmap->Bits(), bitmap->Bits(), bitmap->BitsLength());
		else {
			delete fBitmap;
			fBitmap = NULL;
		}
	}
}

// destructor
BitmapColumnItem::~BitmapColumnItem()
{
	delete fBitmap;
}

// Draw
void				
BitmapColumnItem::Draw(BView* view, Column* column, BRect frame,
					 BRect updateRect, uint32 flags,
					 const column_tree_item_colors* colors)
{
	if (fBitmap && fBitmap->IsValid()) {
		BRect rect(frame.InsetByCopy(1.0f, 1.0f).OffsetToSelf(B_ORIGIN));
		rect = rect & fBitmap->Bounds();
		BPoint bitmapOffset =
			BPoint(8.0f, (int32)(frame.Height() - rect.Height()) / 2)
			+ frame.LeftTop();
/*		BPoint updateOffset(updateRect.LeftTop() - frame.LeftTop());
		rect = rect & updateRect.OffsetToCopy(updateOffset);
*/		if (rect.IsValid()) {
			view->DrawBitmap(fBitmap, rect,
							 rect.OffsetByCopy(bitmapOffset));
		}
	}
}


// TextColumnItem

// constructor
TextColumnItem::TextColumnItem(const char* text)
	: ColumnItem(true),
	  fText(text)
{
}

// destructor
TextColumnItem::~TextColumnItem()
{
}

// Draw
void				
TextColumnItem::Draw(BView* view, Column* column, BRect frame,
					 BRect updateRect, uint32 flags,
					 const column_tree_item_colors* colors)
{
	bool selected = (flags & COLUMN_TREE_ITEM_SELECTED);
	if (selected)
		view->SetHighColor(colors->selected_foreground);
	else
		view->SetHighColor(colors->foreground);
	// draw the title text
	// probably we should...
	// limit the clipping region to the interior of the box
	font_height fontHeight;
	view->GetFontHeight(&fontHeight);
	float ascent = fontHeight.ascent;
	BRect textRect(frame.InsetByCopy(1.0, 1.0));
	BPoint textPoint(textRect.left + 8.0, textRect.top + 2.0 + ascent);

	BString text(fText);
	view->TruncateString(&text, B_TRUNCATE_END, textRect.right - textPoint.x);
	view->SetDrawingMode(B_OP_OVER);
	view->DrawString(text.String(), textPoint);
	view->SetDrawingMode(B_OP_COPY);
}

// DisabledTextColumnItem

// constructor
DisabledTextColumnItem::DisabledTextColumnItem(const char* text)
	: TextColumnItem(text)
{
}

// destructor
DisabledTextColumnItem::~DisabledTextColumnItem()
{
}

// Draw
void				
DisabledTextColumnItem::Draw(BView* view, Column* column, BRect frame,
					 BRect updateRect, uint32 flags,
					 const column_tree_item_colors* colors)
{
	bool selected = (flags & COLUMN_TREE_ITEM_SELECTED);
	rgb_color textColor;
	if (selected) {
		textColor.red = (colors->selected_foreground.red
							+ colors->selected_background.red) / 2;
		textColor.green = (colors->selected_foreground.green
							+ colors->selected_background.green) / 2;
		textColor.blue = (colors->selected_foreground.blue
							+ colors->selected_background.blue) / 2;
	} else {
		textColor.red = (colors->foreground.red
							+ colors->background.red) / 2;
		textColor.green = (colors->foreground.green
							+ colors->background.green) / 2;
		textColor.blue = (colors->foreground.blue
							+ colors->background.blue) / 2;
	}
	view->SetHighColor(textColor);
	// draw the title text
	// probably we should...
	// limit the clipping region to the interior of the box
	font_height fontHeight;
	view->GetFontHeight(&fontHeight);
	float ascent = fontHeight.ascent;
	BRect textRect(frame.InsetByCopy(1.0, 1.0));
	BPoint textPoint(textRect.left + 8.0, textRect.top + 2.0 + ascent);

	BString text(Text());
	view->TruncateString(&text, B_TRUNCATE_END, textRect.right - textPoint.x);
	view->SetDrawingMode(B_OP_OVER);
	view->DrawString(text.String(), textPoint);
	view->SetDrawingMode(B_OP_COPY);
}
