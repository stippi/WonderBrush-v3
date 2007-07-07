// DefaultColumnTreeModel.cpp

#include <algorithm>
#include <new>
#include <stdio.h>

#include "DefaultColumnTreeModel.h"
#include "ColumnTreeItem.h"
#include "ColumnTreeItemCompare.h"

using namespace std;

// Node
struct DefaultColumnTreeModel::Node {
								Node(ColumnTreeModel* model,
									 ColumnTreeItem* item, Node* parent);
								~Node();

			ColumnTreeModel*	GetModel() const		{ return fModel; }
			ColumnTreeItem*		GetItem() const			{ return fItem; }
			Node*				GetParent() const		{ return fParent; }
			int32				GetLevel() const		{ return fLevel; }
			bool				IsRootNode() const		{ return !fItem; }

			void				SetVisible(bool visible);
			bool				IsVisible() const;
			void				SetExpanded(bool expanded);
			bool				IsExpanded() const;
			bool				AreChildrenVisible() const;

			bool				AddChild(Node* child, int32 index);
			Node*				RemoveChild(int32 index);
			bool				RemoveChild(Node* child);
			bool				DeleteChildren(int32 index, int32 count);

			int32				CountChildren() const;
			Node*				ChildAt(int32 index) const;
			int32				IndexOf(Node* item) const;

/*			int32				CountDescendants() const;
			Node*				DescendantAt(int32 index) const;
			int32				IndexOfDescendant(Node* item) const;
*/
			int32				CountVisibleDescendants() const;
			Node*				VisibleDescendantAt(int32 index) const;
			int32				VisibleIndexOfDescendant(Node* item) const;

			void				Sort(SortAdapter& sorter);
			int32				FindSortedInsertionIndex(
									ColumnTreeItem* item,
									const ColumnTreeItemCompare& compare)
									const;

protected:
			ColumnTreeModel*	fModel;
			ColumnTreeItem*		fItem;
			Node*				fParent;
			int32				fLevel;
			BList*				fChildren;
};

// SortAdapter
struct DefaultColumnTreeModel::SortAdapter {
								SortAdapter(ColumnTreeItemCompare* compare)
									: fCompare(compare) {}
								~SortAdapter() {}

			bool				operator()(const Node* node1,
										   const Node* node2)
									{ return (*fCompare)(node1->GetItem(),
														 node2->GetItem()); }

private:
			ColumnTreeItemCompare*	fCompare;
};


// Node

// constructor
DefaultColumnTreeModel::Node::Node(ColumnTreeModel* model,
								   ColumnTreeItem* item, Node* parent)
	: fModel(model),
	  fItem(item),
	  fParent(parent),
	  fLevel(parent ? parent->fLevel + 1 : 0),
	  fChildren(NULL)
{
}

// destructor
DefaultColumnTreeModel::Node::~Node()
{
	delete fChildren;
}

// SetVisible
void
DefaultColumnTreeModel::Node::SetVisible(bool visible)
{
printf("%p->Node::SetVisible(%d)\n", this, visible);
	if (fItem && visible != IsVisible()) {
		const bool childrenVisible = AreChildrenVisible();
		fItem->SetVisible(visible);
		if (childrenVisible != AreChildrenVisible()) {
			for (int32 i = 0; Node* child = ChildAt(i); i++)
				child->SetVisible(!childrenVisible);
		}
	}
}

// IsVisible
bool
DefaultColumnTreeModel::Node::IsVisible() const
{
	return (IsRootNode() || fItem->IsVisible());
}

// SetExpanded
void
DefaultColumnTreeModel::Node::SetExpanded(bool expanded)
{
printf("%p->Node::SetExpanded(%d)\n", this, expanded);
	if (fItem && expanded != IsExpanded()) {
		const bool childrenVisible = AreChildrenVisible();
		fItem->SetExpanded(expanded);
		if (childrenVisible != AreChildrenVisible()) {
			for (int32 i = 0; Node* child = ChildAt(i); i++)
				child->SetVisible(!childrenVisible);
		}
	}
}

// IsExpanded
bool
DefaultColumnTreeModel::Node::IsExpanded() const
{
	return (IsRootNode() || fItem->IsExpanded());
}

// AreChildrenVisible
bool
DefaultColumnTreeModel::Node::AreChildrenVisible() const
{
	return (IsVisible() && IsExpanded());
}

// AddChild
bool
DefaultColumnTreeModel::Node::AddChild(Node* child, int32 index)
{
	// check parameters
	if (!child || index < 0 || index > CountChildren())
		return false;
	// lazy create list
	if (!fChildren) {
		fChildren = new(nothrow) BList(20);
		if (!fChildren)
			return false;
	}
	// add child
	bool result = fChildren->AddItem(child, index);
	if (result)
		child->SetVisible(AreChildrenVisible());
	return result;
}

// RemoveChild
DefaultColumnTreeModel::Node*
DefaultColumnTreeModel::Node::RemoveChild(int32 index)
{
	// check parameters
	if (index < 0 || index >= CountChildren())
		return NULL;
	// remove child
	Node* child = (Node*)fChildren->RemoveItem(index);
	// delete list, if empty and not root node
	if (CountChildren() == 0 && !IsRootNode()) {
		delete fChildren;
		fChildren = NULL;
	}
	return child;
}

// RemoveChild
bool
DefaultColumnTreeModel::Node::RemoveChild(Node* child)
{
	return RemoveChild(IndexOf(child));
}

// DeleteChildren
bool
DefaultColumnTreeModel::Node::DeleteChildren(int32 index, int32 count)
{
	// check parameters
	if (index < 0 || count < 0 || index + count > CountChildren())
		return false;
	if (count == 0)
		return true;
	// delete the child nodes
	for (int32 i = index; i < index + count; i++) {
		Node* child = ChildAt(i);
		child->GetItem()->SetModelData(NULL);
		child->fModel = NULL;
		delete child;
	}
	// delete list, if empty and not root node
	if (CountChildren() == 0 && !IsRootNode()) {
		delete fChildren;
		fChildren = NULL;
	}
	// remove them
	fChildren->RemoveItems(index, count);
	return true;
}

// CountChildren
int32
DefaultColumnTreeModel::Node::CountChildren() const
{
	return (fChildren ? fChildren->CountItems() : 0);
}

// ChildAt
DefaultColumnTreeModel::Node*
DefaultColumnTreeModel::Node::ChildAt(int32 index) const
{
	return (fChildren ? (Node*)fChildren->ItemAt(index) : NULL);
}

// IndexOf
int32
DefaultColumnTreeModel::Node::IndexOf(Node* item) const
{
	return (fChildren ? fChildren->IndexOf(item) : -1);
}

// CountDescendants
/*int32
DefaultColumnTreeModel::Node::CountDescendants() const
{
	int32 count = CountChildren();
	for (int32 i = 0; Node* child = ChildAt(i); i++)
		count += child->CountDescendants();
	return count;
}

// DescendantAt
DefaultColumnTreeModel::Node*
DefaultColumnTreeModel::Node::DescendantAt(int32 index) const
{
// TODO: Optimize! This is horribly inefficient!
	if (index < 0)
		return NULL;
	for (int32 i = 0; Node* child = ChildAt(i); i++) {
		if (index == 0)
			return child;
		index--;
		int32 count = child->CountDescendants();
		if (index < count)
			return child->DescendantAt(index);
		index -= count;
	}
	return NULL;
}

// IndexOfDescendant
int32
DefaultColumnTreeModel::Node::IndexOfDescendant(Node* item) const
{
// TODO: Optimize! This is horribly inefficient!
	if (!item)
		return -1;
	int32 index = 0;
	for (int32 i = 0; Node* child = ChildAt(i); i++) {
		if (child == item)
			return index;
		index++;
		int32 childIndex = child->IndexOfDescendant(item);
		if (childIndex >= 0)
			return index + childIndex;
		index += child->CountDescendants();
	}
	return -1;
}*/

// CountVisibleDescendants
int32
DefaultColumnTreeModel::Node::CountVisibleDescendants() const
{
	if (!AreChildrenVisible())
		return 0;
	int32 count = CountChildren();
	for (int32 i = 0; Node* child = ChildAt(i); i++)
		count += child->CountVisibleDescendants();
	return count;
}

// VisibleDescendantAt
DefaultColumnTreeModel::Node*
DefaultColumnTreeModel::Node::VisibleDescendantAt(int32 index) const
{
// TODO: Optimize! This is horribly inefficient!
	if (index < 0 || !AreChildrenVisible())
		return NULL;
	for (int32 i = 0; Node* child = ChildAt(i); i++) {
		if (index == 0)
			return child;
		index--;
		int32 count = child->CountVisibleDescendants();
		if (index < count)
			return child->VisibleDescendantAt(index);
		index -= count;
	}
	return NULL;
}

// VisibleIndexOfDescendant
int32
DefaultColumnTreeModel::Node::VisibleIndexOfDescendant(Node* item) const
{
// TODO: Optimize! This is horribly inefficient!
	if (!item || !AreChildrenVisible())
		return -1;
	int32 index = 0;
	for (int32 i = 0; Node* child = ChildAt(i); i++) {
		if (child == item)
			return index;
		index++;
		int32 childIndex = child->VisibleIndexOfDescendant(item);
		if (childIndex >= 0)
			return index + childIndex;
		index += child->CountVisibleDescendants();
	}
	return -1;
}

// Sort
void
DefaultColumnTreeModel::Node::Sort(SortAdapter& sorter)
{
	int32 count = CountChildren();
	// sort the children
	if (count >= 2) {
		// copy the items to an array
		Node** children = new Node*[count];
		for (int32 i = 0; i < count; i++)
			children[i] = ChildAt(i);
		sort(children, children + count, sorter);
		// update the items list
		for (int32 i = 0; i < count; i++)
			fChildren->ReplaceItem(i, (void*)children[i]);
		delete[] children;
	}
	// recursively sort the children's descendants
	for (int32 i = 0; i < count; i++)
		ChildAt(i)->Sort(sorter);
}

// FindSortedInsertionIndex
int32
DefaultColumnTreeModel::Node::FindSortedInsertionIndex(
	ColumnTreeItem* item, const ColumnTreeItemCompare& compare) const
{
	int32 index = 0;
	if (item) {
		// binary search
		int32 lower = 0;
		int32 upper = CountChildren();
		while (lower < upper) {
			int32 mid = (lower + upper) / 2;
			Node* midChild = ChildAt(mid);
			if (compare(midChild->GetItem(), item))
				lower = mid + 1;
			else
				upper = mid;
		}
		index = lower;
	}
	return index;
}


// RootNode
struct DefaultColumnTreeModel::RootNode : public DefaultColumnTreeModel::Node {
								RootNode(ColumnTreeModel* model,
										 int32 chunkSize = 100);
								~RootNode() {}
};

// constructor
DefaultColumnTreeModel::RootNode::RootNode(ColumnTreeModel* model,
										   int32 chunkSize)
	: Node(model, NULL, NULL)
{
	if (!fChildren) {
		fChildren = new(nothrow) BList(chunkSize);
	}
}


// DefaultColumnTreeModel

// constructor
DefaultColumnTreeModel::DefaultColumnTreeModel()
	: ColumnTreeModel(COLUMN_TREE_MODEL_SUPPORTS_WRITING
					  | COLUMN_TREE_MODEL_SUPPORTS_SORTING),
	  fRootNode(new(nothrow) RootNode(this, 100)),
	  fItemCount(0)
{
}

// destructor
DefaultColumnTreeModel::~DefaultColumnTreeModel()
{
}

// CountSubItems
int32
DefaultColumnTreeModel::CountSubItems(ColumnTreeItem* item)
{
	if (Node* node = _GetNode(item))
		return node->CountChildren();
	return 0;
}

// SubItemAt
ColumnTreeItem*
DefaultColumnTreeModel::SubItemAt(ColumnTreeItem* super, int32 index)
{
	if (Node* parent = _GetNode(super)) {
		if (Node* node = parent->ChildAt(index))
			return node->GetItem();
	}
	return NULL;
}

// SubItemIndexOf
int32
DefaultColumnTreeModel::SubItemIndexOf(ColumnTreeItem* item)
{
	if (Node* node = _GetNode(item)) {
		if (Node* parent = node->GetParent())
			return parent->IndexOf(node);
	}
	return -1;
}

// SuperItemOf
ColumnTreeItem*
DefaultColumnTreeModel::SuperItemOf(ColumnTreeItem* item)
{
	if (Node* node = _GetNode(item)) {
		if (Node* parent = node->GetParent())
			return parent->GetItem();
	}
	return NULL;
}

// LevelOf
int32
DefaultColumnTreeModel::LevelOf(ColumnTreeItem* item)
{
	if (Node* node = _GetNode(item))
		return node->GetLevel();
	return 0;
}

// CountItems
int32
DefaultColumnTreeModel::CountItems()
{
	return fItemCount;
}

// ItemAt
/*ColumnTreeItem*
DefaultColumnTreeModel::ItemAt(int32 index)
{
	if (index < 0 || index >= fItemCount)
		return NULL;
	if (Node* node = fRootNode->DescendantAt(index))
		return node->GetItem();
	return NULL;
// TODO: improve
}

// IndexOf
int32
DefaultColumnTreeModel::IndexOf(ColumnTreeItem* item)
{
	if (!item)
		return -1;
	if (Node* node = _GetNode(item))
		return fRootNode->IndexOfDescendant(node);
	return -1;
}*/

// HasItem
bool
DefaultColumnTreeModel::HasItem(ColumnTreeItem* item)
{
	if (!item)
		return -1;
	return _GetNode(item);
}

// AddSubItem
bool
DefaultColumnTreeModel::AddSubItem(ColumnTreeItem* super, ColumnTreeItem* item)
{
printf("DefaultColumnTreeModel::AddSubItem(%p, %p)\n", super, item);
	if (!item)
		return false;
	if (Node* parent = _GetNode(super))
		return AddSubItem(super, item, parent->CountChildren());
else
printf("Couldn't get parent node\n");
	return false;
}

// AddSubItem
bool
DefaultColumnTreeModel::AddSubItem(ColumnTreeItem* super, ColumnTreeItem* item,
								   int32 index)
{
printf("DefaultColumnTreeModel::AddSubItem(%p, %p, %ld)\n", super, item, index);
	if (!item)
		return false;
	Node* parent = _GetNode(super);
	if (!parent)
{
printf("  couldn't get parent node\n");
		return false;
}
	// check index
	int32 count = parent->CountChildren();
	if (index < 0 || index > count)
		index = count;
	// if sorted, get insertion idex
	if (GetSortCompareFunction())
{
		index = _FindSortedInsertionIndex(parent, item);
printf("  sorted insertion at: %ld\n", index);
}
	// prepare the item: clear non-user flags
	item->ClearFlags(~COLUMN_TREE_ITEM_USER_FLAGS);
	item->AddFlags(COLUMN_TREE_ITEM_VISIBLE);
	// create a node
	Node* node = new(nothrow) Node(this, item, parent);
	if (!node)
		return false;
	bool result = parent->AddChild(node, index);
	if (result) {
printf("  item successfully added\n");
printf("  parent: expanded: %d, visible: %d\n", parent->IsExpanded(),
parent->IsVisible());
printf("  node:   expanded: %d, visible: %d\n", node->IsExpanded(),
node->IsVisible());
		fItemCount++;
		item->SetModelData(node);
		FireItemsAdded(super, index, 1);
		if (node->IsVisible())
			FireItemsShown(VisibleIndexOf(item), 1);
	} else
{
printf("  parent->AddChild(node, index) failed\n");
		delete node;
}
	return result;
}

// RemoveSubItem
ColumnTreeItem*
DefaultColumnTreeModel::RemoveSubItem(ColumnTreeItem* super, int32 index)
{
	ColumnTreeItem* item = SubItemAt(super, index);
	if (item && RemoveSubItems(super, index, 1))
		return item;
	return  NULL;
}

// RemoveSubItems
bool
DefaultColumnTreeModel::RemoveSubItems(ColumnTreeItem* super, int32 index,
									   int32 count)
{
	// get parent node, check parameters
	Node* parent = _GetNode(super);
	if (parent || index < 0 || count < 0
		|| index + count > parent->CountChildren()) {
		return false;
	}
	if (count == 0)
		return true;
	// recursively remove the nodes' children
	for (int32 i = index + count - 1; i >= index; i--) {
		Node* child = parent->ChildAt(i);
		if (!child)
			return false;
		int32 childCount = child->CountChildren();
		if (childCount > 0
			&& !RemoveSubItems(child->GetItem(), 0, childCount)) {
			return false;
		}
	}
	// visibility info
	bool visible = parent->AreChildrenVisible();
	int32 visibleIndex = -1;
	if (visible)
		visibleIndex = VisibleIndexOf(parent->ChildAt(index)->GetItem());
	// remove the nodes and trigger notifications
	if (visible)
		FireItemsHidden(visibleIndex, count, true);
	FireItemsRemoved(super, index, count, true);
	parent->DeleteChildren(index, count);
	fItemCount -= count;
	if (visible)
		FireItemsHidden(visibleIndex, count, false);
	FireItemsRemoved(super, index, count, false);
	return true;
}

// RemoveItem
bool
DefaultColumnTreeModel::RemoveItem(ColumnTreeItem* item)
{
	if (!item)
		return false;
	if (Node* node = _GetNode(item)) {
		if (Node* parent = node->GetParent())
			return RemoveSubItems(parent->GetItem(), parent->IndexOf(node), 1);
	}
	return false;
}

// MakeEmpty
bool
DefaultColumnTreeModel::MakeEmpty()
{
	int32 count = CountSubItems(NULL);
	if (count > 0)
		return RemoveSubItems(NULL, 0, count);
	return true;
}

// CountVisibleItems
int32
DefaultColumnTreeModel::CountVisibleItems()
{
	return fRootNode->CountVisibleDescendants();
}

// VisibleItemAt
ColumnTreeItem*
DefaultColumnTreeModel::VisibleItemAt(int32 index)
{
	if (index < 0 || index > fItemCount)
		return NULL;
	if (Node* node = fRootNode->VisibleDescendantAt(index))
		return node->GetItem();
	return NULL;
}

// VisibleIndexOf
int32
DefaultColumnTreeModel::VisibleIndexOf(ColumnTreeItem* item)
{
	if (!item || !item->IsVisible())
		return -1;
	if (Node* node = _GetNode(item))
		return fRootNode->VisibleIndexOfDescendant(node);
	return -1;
}

// CollapseItem
bool
DefaultColumnTreeModel::CollapseItem(ColumnTreeItem* item)
{
	// get node, check parameters
	if (!item)
		return false;
	Node* node = _GetNode(item);
	if (!node)
		return false;
	if (!node->IsExpanded())
		return true;
	// collapse the node
	bool visible = node->IsVisible();
	int32 count = node->CountVisibleDescendants();
	if (visible && count > 0)
		FireItemsHidden(VisibleIndexOf(item) + 1, count, true);
	node->SetExpanded(false);
	if (visible && count > 0)
		FireItemsHidden(VisibleIndexOf(item) + 1, count, false);
	FireItemCollapsed(item);
	return true;
}

// CollapseItem
bool
DefaultColumnTreeModel::CollapseItem(ColumnTreeItem* super, int32 index)
{
	return CollapseItem(SubItemAt(super, index));
}

// CollapseVisibleItem
bool
DefaultColumnTreeModel::CollapseVisibleItem(int32 index)
{
	return CollapseItem(VisibleItemAt(index));
}

// ExpandItem
bool
DefaultColumnTreeModel::ExpandItem(ColumnTreeItem* item)
{
	// get node, check parameters
	if (!item)
		return false;
	Node* node = _GetNode(item);
	if (!node || !node->IsVisible())
		return false;
	if (node->IsExpanded())
		return true;
	// expand the node
	node->SetExpanded(true);
	if (node->IsVisible()) {
		int32 count = node->CountVisibleDescendants();
		if (count > 0)
			FireItemsShown(VisibleIndexOf(item) + 1, count);
	}
	FireItemExpanded(item);
	return true;
}

// ExpandItem
bool
DefaultColumnTreeModel::ExpandItem(ColumnTreeItem* super, int32 index)
{
	return ExpandItem(SubItemAt(super, index));
}

// ExpandVisibleItem
bool
DefaultColumnTreeModel::ExpandVisibleItem(int32 index)
{
	return ExpandItem(VisibleItemAt(index));
}

// SortCompareFunctionChanged
void
DefaultColumnTreeModel::SortCompareFunctionChanged()
{
	if (GetSortCompareFunction())
		_Sort();
}

// ItemChanged
void
DefaultColumnTreeModel::ItemChanged(ColumnTreeItem* item)
{
}

// _GetNode
DefaultColumnTreeModel::Node*
DefaultColumnTreeModel::_GetNode(ColumnTreeItem* item)
{
	if (!item)
		return fRootNode;
	if (Node* node = (Node*)item->GetModelData()) {
		if (node->GetModel() == this)
			return node;
	}
	return NULL;
}

// _Sort
void
DefaultColumnTreeModel::_Sort()
{
	if (GetSortCompareFunction()) {
		SortAdapter sorter(GetSortCompareFunction());
		fRootNode->Sort(sorter);
		FireItemsSorted();
	}
}

// _FindSortedInsertionIndex
//
// Finds the index in the items list, /item/ has to be inserted according
// to the current sort compare function. The item is inserted after all
// equal items.
int32
DefaultColumnTreeModel::_FindSortedInsertionIndex(Node* parent,
												  ColumnTreeItem* item)
{
	const ColumnTreeItemCompare* compare = GetSortCompareFunction();
	if (!parent || !compare)
		return 0;
	return parent->FindSortedInsertionIndex(item, *compare);
}

