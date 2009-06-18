// ColumnTreeModel.h

#ifndef COLUMN_TREE_MODEL_H
#define COLUMN_TREE_MODEL_H

#include <List.h>

// flags indicating, what features the model supports
enum {
	COLUMN_TREE_MODEL_SUPPORTS_WRITING		= 0x01,
	COLUMN_TREE_MODEL_SUPPORTS_VISIBILITY	= 0x02,
	COLUMN_TREE_MODEL_SUPPORTS_SORTING		= 0x04,
};

class ColumnTreeItem;
class ColumnTreeItemCompare;
class ColumnTreeModelListener;

class ColumnTreeModel {
public:
								ColumnTreeModel(uint32 flags);
	virtual						~ColumnTreeModel();

			bool				SupportsWriting() const;
			bool				SupportsVisibility() const;
			bool				SupportsSorting() const;

	// general read-only functionality (required)

	virtual	int32				CountSubItems(ColumnTreeItem* super) = 0;
	virtual	ColumnTreeItem*		SubItemAt(ColumnTreeItem* super,
										  int32 index) = 0;
	virtual	int32				SubItemIndexOf(ColumnTreeItem* item) = 0;
	virtual	ColumnTreeItem*		SuperItemOf(ColumnTreeItem* item) = 0;
	virtual	int32				LevelOf(ColumnTreeItem* item) = 0;

	virtual	int32				CountItems() = 0;
	virtual	bool				HasItem(ColumnTreeItem* item) = 0;

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

	// visibility related functionality (required)

	virtual	int32				CountVisibleItems() = 0;
	virtual	ColumnTreeItem*		VisibleItemAt(int32 index) = 0;
	virtual	int32				VisibleIndexOf(ColumnTreeItem* item) = 0;

	// visibility related functionality (optional)

	virtual	bool				CollapseItem(ColumnTreeItem* item);
	virtual	bool				CollapseItem(ColumnTreeItem* super,
											 int32 index);
	virtual	bool				CollapseVisibleItem(int32 index);
	virtual	bool				ExpandItem(ColumnTreeItem* item);
	virtual	bool				ExpandItem(ColumnTreeItem* super,
										   int32 index);
	virtual	bool				ExpandVisibleItem(int32 index);

	// sorting functionality (optional)

			void				SetSortCompareFunction(
									ColumnTreeItemCompare* compare);
			ColumnTreeItemCompare*	GetSortCompareFunction() const;
	virtual	void				SortCompareFunctionChanged();
	virtual	void				ItemChanged(ColumnTreeItem* item);

	// listener functionality

	virtual	void				AddListener(ColumnTreeModelListener *listener);
	virtual	void				RemoveListener(
									ColumnTreeModelListener *listener);

	virtual	ColumnTreeModelListener*	ListenerAt(int32 index);

	virtual	void				FireItemsAdded(ColumnTreeItem* super,
											   int32 index, int32 count);
	virtual	void				FireItemsRemoved(ColumnTreeItem* super,
												 int32 index, int32 count,
												 bool before);

	virtual	void				FireItemExpanded(ColumnTreeItem* item);
	virtual	void				FireItemCollapsed(ColumnTreeItem* item);

	virtual	void				FireItemsShown(int32 index, int32 count);
	virtual	void				FireItemsHidden(int32 index, int32 count,
												bool before);

	virtual	void				FireItemsSorted();

private:
			BList				fListeners;
			uint32				fFlags;
			ColumnTreeItemCompare*	fCompareFunction;
};

#endif	// COLUMN_TREE_MODEL_H
