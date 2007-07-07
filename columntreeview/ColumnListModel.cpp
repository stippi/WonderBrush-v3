// ColumnListModel.cpp

#include <algorithm>

#include "ColumnListModel.h"
#include "ColumnTreeItem.h"
#include "ColumnTreeItemCompare.h"

// SortAdapter
struct ColumnListModel::SortAdapter {
								SortAdapter(ColumnTreeItemCompare* compare)
									: fCompare(compare) {}
								~SortAdapter() {}

			bool				operator()(const ColumnTreeItem* item1,
										   const ColumnTreeItem* item2)
									{ return (*fCompare)(item1, item2); }

private:
			ColumnTreeItemCompare*	fCompare;
};

// constructor
ColumnListModel::ColumnListModel()
	: ColumnTreeModel(COLUMN_TREE_MODEL_SUPPORTS_WRITING
					  | COLUMN_TREE_MODEL_SUPPORTS_SORTING),
	  fItems(100)
{
}

// destructor
ColumnListModel::~ColumnListModel()
{
}

// CountSubItems
int32
ColumnListModel::CountSubItems(ColumnTreeItem* super)
{
	if (super)
		return 0;
	return fItems.CountItems();
}

// SubItemAt
ColumnTreeItem*
ColumnListModel::SubItemAt(ColumnTreeItem* super, int32 index)
{
	if (super)
		return NULL;
	return (ColumnTreeItem*)fItems.ItemAt(index);
}

// IndexOf
int32
ColumnListModel::SubItemIndexOf(ColumnTreeItem* item)
{
	return IndexOf(item);
}

// SuperItemOf
ColumnTreeItem*
ColumnListModel::SuperItemOf(ColumnTreeItem* item)
{
	return NULL;
}

// LevelOf
int32
ColumnListModel::LevelOf(ColumnTreeItem* item)
{
	return 0;
}

// CountItems
int32
ColumnListModel::CountItems()
{
	return fItems.CountItems();
}

// ItemAt
ColumnTreeItem*
ColumnListModel::ItemAt(int32 index)
{
	return (ColumnTreeItem*)fItems.ItemAt(index);
}

// IndexOf
int32
ColumnListModel::IndexOf(ColumnTreeItem* item)
{
	return fItems.IndexOf(item);
}

// HasItem
bool
ColumnListModel::HasItem(ColumnTreeItem* item)
{
	return fItems.HasItem(item);
}

int32
ColumnListModel::CountVisibleItems()
{
	return CountItems();
}

// VisibleItemAt
ColumnTreeItem*
ColumnListModel::VisibleItemAt(int32 index)
{
	return ItemAt(index);
}

// VisibleIndexOf
int32
ColumnListModel::VisibleIndexOf(ColumnTreeItem* item)
{
	return IndexOf(item);
}

// AddSubItem
bool
ColumnListModel::AddSubItem(ColumnTreeItem* super, ColumnTreeItem* item)
{
	if (super || !item)
		return false;
	return AddSubItem(super, item, CountItems());
}

// AddSubItem
bool
ColumnListModel::AddSubItem(ColumnTreeItem* super, ColumnTreeItem* item,
							int32 index)
{
	if (super || !item)
		return false;
	// check index
	int32 count = CountItems();
	if (index < 0 || index > count)
		index = count;
	// if sorted, get insertion idex
	if (GetSortCompareFunction())
		index = _FindSortedInsertionIndex(item);
	// prepare the item: clear non-user flags, set level/parent
	item->ClearFlags(~COLUMN_TREE_ITEM_USER_FLAGS);
	item->AddFlags(COLUMN_TREE_ITEM_VISIBLE);
	bool result = fItems.AddItem(item, index);
	if (result) {
		FireItemsAdded(super, index, 1);
		FireItemsShown(index, 1);
	}
	return result;
}

// RemoveSubItem
ColumnTreeItem*
ColumnListModel::RemoveSubItem(ColumnTreeItem* super, int32 index)
{
	if (super)
		return NULL;
	ColumnTreeItem* item = (ColumnTreeItem*)fItems.ItemAt(index);
	if (item) {
		FireItemsHidden(index, 1, true);
		FireItemsRemoved(super, index, 1, true);
		fItems.RemoveItem(index);
		FireItemsHidden(index, 1, false);
		FireItemsRemoved(super, index, 1, false);
	}
	return item;
}

// RemoveSubItems
bool
ColumnListModel::RemoveSubItems(ColumnTreeItem* super, int32 index,
								int32 count)
{
	if (super || index < 0 || count < 0 || index + count > CountItems())
		return false;
	if (count == 0)
		return true;
	FireItemsHidden(index, count, true);
	FireItemsRemoved(super, index, count, true);
	fItems.RemoveItems(index, count);
	FireItemsHidden(index, count, false);
	FireItemsRemoved(super, index, count, false);
	return true;
}

// RemoveItem
bool
ColumnListModel::RemoveItem(ColumnTreeItem* item)
{
	if (!item)
		return false;
	return RemoveSubItem(NULL, IndexOf(item));
}

// MakeEmpty
bool
ColumnListModel::MakeEmpty()
{
	int32 count = CountItems();
	if (count > 0) {
		FireItemsHidden(0, count, true);
		FireItemsRemoved(NULL, 0, count, true);
		fItems.MakeEmpty();
		FireItemsHidden(0, count, false);
		FireItemsRemoved(NULL, 0, count, false);
	}
	return true;
}

// SortCompareFunctionChanged
void
ColumnListModel::SortCompareFunctionChanged()
{
	if (GetSortCompareFunction())
		_Sort();
}

// ItemChanged
void
ColumnListModel::ItemChanged(ColumnTreeItem* item)
{
/*	if (ColumnTreeItem* item = ItemAt(index)) {
		if (fCompareFunction && (fPrimarySortColumn || fSecondarySortColumn)) {
			int32 count = CountItems();
			ItemGreater greater(fCompareFunction, fPrimarySortColumn,
								fSecondarySortColumn);
			// compare the item with its predecessor and successor
			if (index > 0 && greater(ItemAt(index - 1), item)
				|| index < count - 1
				   && greater(item, ItemAt(index + 1)) > 0) {
				// the item is misplaced -- remove and reinsert it
				bool selected = item->IsSelected();
				RemoveItem(index);
				AddItem(item, index);
				if (selected)
					Select(IndexOf(item));
			}
		}
		InvalidateItem(item);
	}
*/
}

// _Sort
void
ColumnListModel::_Sort()
{
	int32 count = CountItems();
	if (!GetSortCompareFunction() || count < 2)
		return;
	SortAdapter sorter(GetSortCompareFunction());
	// copy the items to an array
	ColumnTreeItem** items = new ColumnTreeItem*[count];
	for (int32 i = 0; i < count; i++)
		items[i] = ItemAt(i);
	sort(items, items + count, sorter);
	// update the items list
	for (int32 i = 0; i < count; i++)
		fItems.ReplaceItem(i, (void*)items[i]);
	delete[] items;
	FireItemsSorted();
}

// _FindSortedInsertionIndex
//
// Finds the index in the items list, /item/ has to be inserted according
// to the current sort compare function. The item is inserted after all
// equal items.
int32
ColumnListModel::_FindSortedInsertionIndex(ColumnTreeItem* item)
{
	int32 index = 0;
	if (item && GetSortCompareFunction()) {
		const ColumnTreeItemCompare& compare = *GetSortCompareFunction();
		// binary search
		int32 lower = 0;
		int32 upper = CountItems();
		while (lower < upper) {
			int32 mid = (lower + upper) / 2;
			ColumnTreeItem* midItem = ItemAt(mid);
			if (compare(midItem, item))
				lower = mid + 1;
			else
				upper = mid;
		}
		index = lower;
	}
	return index;
}

