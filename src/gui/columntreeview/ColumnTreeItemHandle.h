// ColumnTreeItemHandle.h

#ifndef COLUMN_TREE_ITEM_HANDLE_H
#define COLUMN_TREE_ITEM_HANDLE_H

#include <Rect.h>

class BView;
class Column;
class ColumnTreeItem;
class ColumnTreeModel;
struct column_tree_item_colors;

class ColumnTreeItemHandle {
public:
								ColumnTreeItemHandle();
	virtual						~ColumnTreeItemHandle();

	virtual	void				SetModel(ColumnTreeModel* model);

	virtual	float				GetIndentation(int32 level) = 0;

	virtual	BRect				GetHandleRect(ColumnTreeItem* item,
											  BRect frame) = 0;

	virtual	void				Draw(BView* view, ColumnTreeItem* item,
									 Column* column, BRect frame,
									 BRect updateRect, uint32 flags,
									 const column_tree_item_colors* colors)
									 = 0;

protected:
			ColumnTreeModel*	fModel;
};

#endif	// COLUMN_TREE_ITEM_HANDLE_H
