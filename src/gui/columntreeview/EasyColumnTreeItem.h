// EasyColumnTreeItem.h

#ifndef EASY_COLUMN_TREE_ITEM_H
#define EASY_COLUMN_TREE_ITEM_H

#include <List.h>

#include "ColumnTreeItem.h"

class BBitmap;

class ColumnItem;

class EasyColumnTreeItem : public ColumnTreeItem {
 public:
								EasyColumnTreeItem(float height);
	virtual						~EasyColumnTreeItem();

	virtual	void				Draw(BView* view, Column* column, BRect frame,
									 BRect updateRect, uint32 flags,
									 const column_tree_item_colors* colors);

			void				SetContent(int32 index, ColumnItem* item);
			ColumnItem*			ColumnItemAt(int32 index) const;

	// convenience methods
			void				SetContent(int32 index, const BBitmap* bitmap);
			void				SetContent(int32 index, const char* text,
										   bool disabled = false);

	static	int 				StandardCompare(
										const ColumnTreeItem* item1,
										const ColumnTreeItem* item2,
										const Column* column);


 private:
			BList				fColumnItems;
};


#endif	// EASY_COLUMN_TREE_ITEM_H
