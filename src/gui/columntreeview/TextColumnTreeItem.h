// TextColumnTreeItem.h

#ifndef TEXT_COLUMN_TREE_ITEM_H
#define TEXT_COLUMN_TREE_ITEM_H

#include "ColumnTreeItem.h"

class TextColumnTreeItem : public ColumnTreeItem {
 public:
								TextColumnTreeItem(float height);
	virtual						~TextColumnTreeItem();

	virtual	void				Draw(BView* view, Column* column, BRect frame,
									 BRect updateRect, uint32 flags,
									 const column_tree_item_colors* colors);

			void				SetText(const char* text, int32 index);
			const char*			TextAt(int32 index) const;

 private:
			BString*			fTexts;
			int32				fColumnCount;
};


#endif	// TEXT_COLUMN_TREE_ITEM_H
