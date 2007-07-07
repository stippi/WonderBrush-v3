// ColumnTreeItemCompare.h

#ifndef COLUMN_TREE_ITEM_COMPARE_H
#define COLUMN_TREE_ITEM_COMPARE_H

class ColumnTreeItem;

class ColumnTreeItemCompare {
public:
								ColumnTreeItemCompare();
	virtual						~ColumnTreeItemCompare();

	virtual	bool				operator()(const ColumnTreeItem* item1,
										   const ColumnTreeItem* item1)
									const = 0;
};

#endif	// COLUMN_TREE_ITEM_COMPARE_H
