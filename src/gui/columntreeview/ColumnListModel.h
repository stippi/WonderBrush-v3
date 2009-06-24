// ColumnListModel.h

#ifndef COLUMN_LIST_MODEL_H
#define COLUMN_LIST_MODEL_H

#include <List.h>

#include "ColumnTreeModel.h"

class ColumnListModel : public ColumnTreeModel {
public:
								ColumnListModel();
	virtual						~ColumnListModel();

	// general read-only functionality (required)

	virtual	int32				CountSubItems(ColumnTreeItem* super);
	virtual	ColumnTreeItem*		SubItemAt(ColumnTreeItem* super, int32 index);
	virtual	int32				SubItemIndexOf(ColumnTreeItem* item);
	virtual	ColumnTreeItem*		SuperItemOf(ColumnTreeItem* item);
	virtual	int32				LevelOf(ColumnTreeItem* item);

	virtual	int32				CountItems();
			ColumnTreeItem*		ItemAt(int32 index);
			int32				IndexOf(ColumnTreeItem* item);
	virtual	bool				HasItem(ColumnTreeItem* item);

	// visibility related functionality (required)

	virtual	int32				CountVisibleItems();
	virtual	ColumnTreeItem*		VisibleItemAt(int32 index);
	virtual	int32				VisibleIndexOf(ColumnTreeItem* item);

	// general write functionality (optional)

	virtual	bool				AddSubItem(ColumnTreeItem* super,
										   ColumnTreeItem* item);
	virtual	bool				AddSubItem(ColumnTreeItem* super,
										   ColumnTreeItem* item,
										   int32 index);
	virtual	ColumnTreeItem*		RemoveSubItem(ColumnTreeItem* super,
											  int32 index);
	virtual	bool				RemoveSubItems(ColumnTreeItem* super,
											   int32 index, int32 count);
	virtual	bool				RemoveItem(ColumnTreeItem* item);
	virtual	bool				MakeEmpty();

	// sorting functionality (optional)

	virtual	void				SortCompareFunctionChanged();
	virtual	void				ItemChanged(ColumnTreeItem* item);

private:
	struct SortAdapter;

private:

			void				_Sort();
			int32				_FindSortedInsertionIndex(
									ColumnTreeItem* item);

private:
	BList						fItems;
	BList						fVisibleItems;
};

#endif	// COLUMN_LIST_MODEL_H
