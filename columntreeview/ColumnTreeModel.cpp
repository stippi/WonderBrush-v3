// ColumnTreeModel.cpp

#include "ColumnTreeModel.h"
#include "ColumnTreeModelListener.h"

// constructor
ColumnTreeModel::ColumnTreeModel(uint32 flags)
	: fListeners(10),
	  fFlags(flags),
	  fCompareFunction(NULL)
{
}

// destructor
ColumnTreeModel::~ColumnTreeModel()
{
}

// SupportsWriting
bool
ColumnTreeModel::SupportsWriting() const
{
	return (fFlags & COLUMN_TREE_MODEL_SUPPORTS_WRITING);
}

// SupportsVisibility
bool
ColumnTreeModel::SupportsVisibility() const
{
	return (fFlags & COLUMN_TREE_MODEL_SUPPORTS_VISIBILITY);
}

// SupportsSorting
bool
ColumnTreeModel::SupportsSorting() const
{
	return (fFlags & COLUMN_TREE_MODEL_SUPPORTS_SORTING);
}

// AddSubItem
bool
ColumnTreeModel::AddSubItem(ColumnTreeItem* super, ColumnTreeItem* item)
{
	return false;
}

// AddSubItem
bool
ColumnTreeModel::AddSubItem(ColumnTreeItem* super, ColumnTreeItem* item,
							int32 index)
{
	return false;
}

// RemoveSubItem
ColumnTreeItem*
ColumnTreeModel::RemoveSubItem(ColumnTreeItem* super, int32 index)
{
	return NULL;
}

// RemoveSubItems
bool
ColumnTreeModel::RemoveSubItems(ColumnTreeItem* super, int32 index,
								int32 count)
{
	return false;
}

// RemoveItem
bool
ColumnTreeModel::RemoveItem(ColumnTreeItem* item)
{
	return false;
}

// MakeEmpty
bool
ColumnTreeModel::MakeEmpty()
{
	return false;
}

// CollapseItem
bool
ColumnTreeModel::CollapseItem(ColumnTreeItem* item)
{
	return false;
}

// CollapseItem
bool
ColumnTreeModel::CollapseItem(ColumnTreeItem* super, int32 index)
{
	return false;
}

// CollapseVisibleItem
bool
ColumnTreeModel::CollapseVisibleItem(int32 index)
{
	return false;
}

// ExpandItem
bool
ColumnTreeModel::ExpandItem(ColumnTreeItem* item)
{
	return false;
}

// ExpandItem
bool
ColumnTreeModel::ExpandItem(ColumnTreeItem* super, int32 index)
{
	return false;
}

// ExpandVisibleItem
bool
ColumnTreeModel::ExpandVisibleItem(int32 index)
{
	return false;
}

// SetSortCompareFunction
void
ColumnTreeModel::SetSortCompareFunction(ColumnTreeItemCompare* compare)
{
	if (fCompareFunction != compare) {
		fCompareFunction = compare;
		SortCompareFunctionChanged();
	}
}

// GetSortCompareFunction
ColumnTreeItemCompare*
ColumnTreeModel::GetSortCompareFunction() const
{
	return fCompareFunction;
}

// SortCompareFunctionChanged
void
ColumnTreeModel::SortCompareFunctionChanged()
{
}

// ItemChanged
//
// To be called, when an item has changed. The sort position is checked
// and the item is invalidated.
void
ColumnTreeModel::ItemChanged(ColumnTreeItem* item)
{
}

// AddListener
void
ColumnTreeModel::AddListener(ColumnTreeModelListener *listener)
{
	if (!listener || fListeners.HasItem(listener))
		return;
	fListeners.AddItem(listener);
}

// RemoveListener
void
ColumnTreeModel::RemoveListener(ColumnTreeModelListener *listener)
{
	if (listener)
		fListeners.RemoveItem(listener);
}

// ListenerAt
ColumnTreeModelListener*
ColumnTreeModel::ListenerAt(int32 index)
{
	return (ColumnTreeModelListener*)fListeners.ItemAt(index);
}

// FireItemsAdded
void
ColumnTreeModel::FireItemsAdded(ColumnTreeItem* super, int32 index,
								int32 count)
{
	for (int32 i = 0; ColumnTreeModelListener* listener = ListenerAt(i); i++)
		listener->ItemsAdded(this, super, index, count);
}

// FireItemsRemoved
void
ColumnTreeModel::FireItemsRemoved(ColumnTreeItem* super, int32 index,
								  int32 count, bool before)
{
	for (int32 i = 0; ColumnTreeModelListener* listener = ListenerAt(i); i++)
		listener->ItemsRemoved(this, super, index, count, before);
}

// FireItemExpanded
void
ColumnTreeModel::FireItemExpanded(ColumnTreeItem* item)
{
	for (int32 i = 0; ColumnTreeModelListener* listener = ListenerAt(i); i++)
		listener->ItemExpanded(this, item);
}

// FireItemCollapsed
void
ColumnTreeModel::FireItemCollapsed(ColumnTreeItem* item)
{
	for (int32 i = 0; ColumnTreeModelListener* listener = ListenerAt(i); i++)
		listener->ItemCollapsed(this, item);
}

// FireItemsShown
void
ColumnTreeModel::FireItemsShown(int32 index, int32 count)
{
	for (int32 i = 0; ColumnTreeModelListener* listener = ListenerAt(i); i++)
		listener->ItemsShown(this, index, count);
}

// FireItemsHidden
void
ColumnTreeModel::FireItemsHidden(int32 index, int32 count, bool before)
{
	for (int32 i = 0; ColumnTreeModelListener* listener = ListenerAt(i); i++)
		listener->ItemsHidden(this, index, count, before);
}

// FireItemsSorted
void
ColumnTreeModel::FireItemsSorted()
{
	for (int32 i = 0; ColumnTreeModelListener* listener = ListenerAt(i); i++)
		listener->ItemsSorted(this);
}

