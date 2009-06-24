// DefaultColumnTreeModel.h

#ifndef COLUMN_LIST_MODEL_H
#define COLUMN_LIST_MODEL_H

#include <List.h>

#include "ColumnTreeModel.h"

class DefaultColumnTreeModel : public ColumnTreeModel {
public:
								DefaultColumnTreeModel();
	virtual						~DefaultColumnTreeModel();

	// general read-only functionality (required)

	virtual	int32				CountSubItems(ColumnTreeItem* super);
	virtual	ColumnTreeItem*		SubItemAt(ColumnTreeItem* super, int32 index);
	virtual	int32				SubItemIndexOf(ColumnTreeItem* item);
	virtual	ColumnTreeItem*		SuperItemOf(ColumnTreeItem* item);
	virtual	int32				LevelOf(ColumnTreeItem* item);

	virtual	int32				CountItems();
//			ColumnTreeItem*		ItemAt(int32 index);
//			int32				IndexOf(ColumnTreeItem* item);
	virtual	bool				HasItem(ColumnTreeItem* item);

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

	// visibility related functionality (optional)

	virtual	int32				CountVisibleItems();
	virtual	ColumnTreeItem*		VisibleItemAt(int32 index);
	virtual	int32				VisibleIndexOf(ColumnTreeItem* item);

	virtual	bool				CollapseItem(ColumnTreeItem* item);
	virtual	bool				CollapseItem(ColumnTreeItem* super,
											 int32 index);
	virtual	bool				CollapseVisibleItem(int32 index);
	virtual	bool				ExpandItem(ColumnTreeItem* item);
	virtual	bool				ExpandItem(ColumnTreeItem* super,
										   int32 index);
	virtual	bool				ExpandVisibleItem(int32 index);

	// sorting functionality (optional)

	virtual	void				SortCompareFunctionChanged();
	virtual	void				ItemChanged(ColumnTreeItem* item);

private:
	struct SortAdapter;
	struct Node;
	struct RootNode;

private:
			Node*				_GetNode(ColumnTreeItem* item);
			void				_Sort();
			int32				_FindSortedInsertionIndex(Node* parent,
									ColumnTreeItem* item);

private:
	RootNode*					fRootNode;
//	BList						fVisibleItems;
	int32						fItemCount;
};

#endif	// COLUMN_LIST_MODEL_H
