// TextColumnTreeItem.cpp

#include <String.h>
#include <View.h>

#include "TextColumnTreeItem.h"
#include "Column.h"
#include "ColumnTreeViewColors.h"


// constructor
TextColumnTreeItem::TextColumnTreeItem(float height)
	: ColumnTreeItem(height),
	  fTexts(NULL),
	  fColumnCount(0)
{
}

// destructor
TextColumnTreeItem::~TextColumnTreeItem()
{
}

// Draw
void
TextColumnTreeItem::Draw(BView* view, Column* column, BRect frame,
						 BRect updateRect, uint32 flags,
						 const column_tree_item_colors* colors)
{
	bool selected = false;
	DrawBackground(view, column, frame, updateRect, flags, colors);
	if (selected)
		view->SetHighColor(colors->selected_foreground);
	else
		view->SetHighColor(colors->foreground);
	// draw the title text
	// probably we should...
	// limit the clipping region to the interior of the box
	font_height fontHeight;
	view->GetFontHeight(&fontHeight);
	BRect textRect(frame.InsetByCopy(1.0, 1.0));
	BPoint textPoint(textRect.left + 8.0, textRect.bottom - 2.0 - fontHeight.descent);

	BString text;
	if (column->Index() < fColumnCount)
		text.SetTo(fTexts[column->Index()]);
//	BString text(fText); text << column->Index();
	view->TruncateString(&text, B_TRUNCATE_END, textRect.right - textPoint.x);
	view->SetDrawingMode(B_OP_OVER);
	view->DrawString(text.String(), textPoint);
	view->SetDrawingMode(B_OP_COPY);
}

// SetText
void
TextColumnTreeItem::SetText(const char* text, int32 index)
{
	if (index >= 0) {
		// realloc the texts array if necessary
		if (index >= fColumnCount) {
			BString* texts = new BString[index + 1];
			for (int32 i = 0; i < fColumnCount; i++)
				texts[i] = fTexts[i];
			delete[] fTexts;
			fTexts = texts;
			fColumnCount = index + 1;
		}
		// set the text
		fTexts[index].SetTo(text);
	}
}

// TextAt
const char*
TextColumnTreeItem::TextAt(int32 index) const
{
	if (index >= 0 && index < fColumnCount)
		return fTexts[index].String();
	return NULL;
}

