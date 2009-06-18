// EasyColumnTreeItem.cpp

#include <stdio.h>
#include <string.h>

#include <String.h>
#include <View.h>

#include "EasyColumnTreeItem.h"
#include "Column.h"
#include "ColumnItem.h"
#include "ColumnTreeViewColors.h"


// constructor
EasyColumnTreeItem::EasyColumnTreeItem(float height)
	: ColumnTreeItem(height),
	  fColumnItems(10)
{
}

// destructor
EasyColumnTreeItem::~EasyColumnTreeItem()
{
	for (int32 i = 0; ColumnItem* item = ColumnItemAt(i); i++)
		delete item;
}

// ColumnItemAt
ColumnItem*
EasyColumnTreeItem::ColumnItemAt(int32 index) const
{
	return (ColumnItem*)fColumnItems.ItemAt(index);
}


// Draw
void
EasyColumnTreeItem::Draw(BView* view, Column* column, BRect frame,
						 BRect updateRect, uint32 flags,
						 const column_tree_item_colors* colors)
{
	ColumnItem* item = NULL;
	if (column)
		item = ColumnItemAt(column->Index());
	if (!item || item->ClearBackground())
		DrawBackground(view, column, frame, updateRect, flags, colors);
	if (item)
		item->Draw(view, column, frame, updateRect, flags, colors);
}

// SetContent
void
EasyColumnTreeItem::SetContent(int32 index, ColumnItem* item)
{
	if (item) {
		int32 count = fColumnItems.CountItems();
		// check index
		if (index < 0)
			index = count;
		// insert dummy items, if necessary
		for (int32 i = count; i < index; i++)
			fColumnItems.AddItem((void*)new ColumnItem(true), i);
		// delete old item and insert new item
		if (ColumnItem* oldItem = (ColumnItem*)fColumnItems.RemoveItem(index))
			delete oldItem;
		fColumnItems.AddItem((void*)item, index);
	}
}

// SetContent
void
EasyColumnTreeItem::SetContent(int32 index, const BBitmap* bitmap)
{
	SetContent(index, new BitmapColumnItem(bitmap));
}

// SetContent
void
EasyColumnTreeItem::SetContent(int32 index, const char* text, bool disabled)
{
	if (disabled)
		SetContent(index, new DisabledTextColumnItem(text));
	else
		SetContent(index, new TextColumnItem(text));
}

// StandardCompare
int
EasyColumnTreeItem::StandardCompare(const ColumnTreeItem* item1,
											const ColumnTreeItem* item2,
											const Column* column)
{
	const EasyColumnTreeItem* it1 =
		dynamic_cast<const EasyColumnTreeItem*>(item1);
	const EasyColumnTreeItem* it2 =
		dynamic_cast<const EasyColumnTreeItem*>(item2);
	if (it1 && it2) {
		TextColumnItem* citem1 =
			dynamic_cast<TextColumnItem*>(it1->ColumnItemAt(column->Index()));
		TextColumnItem* citem2 =
			dynamic_cast<TextColumnItem*>(it2->ColumnItemAt(column->Index()));
		if (citem1 && citem2)
			return strcmp(citem1->Text(), citem2->Text());
	}
	return 0;
}
