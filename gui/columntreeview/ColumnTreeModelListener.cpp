// ColumnTreeModelListener.cpp

#include "ColumnTreeModelListener.h"

// constructor
ColumnTreeModelListener::ColumnTreeModelListener()
{
}

// destructor
ColumnTreeModelListener::~ColumnTreeModelListener()
{
}

// ItemsAdded
void
ColumnTreeModelListener::ItemsAdded(ColumnTreeModel* model,
									ColumnTreeItem* super, int32 index,
									int32 count)
{
}

// ItemsRemoved
void
ColumnTreeModelListener::ItemsRemoved(ColumnTreeModel* model,
									  ColumnTreeItem* super, int32 index,
									  int32 count, bool before)
{
}

// ItemExpanded
void
ColumnTreeModelListener::ItemExpanded(ColumnTreeModel* model,
									  ColumnTreeItem* item)
{
}

// ItemCollapsed
void
ColumnTreeModelListener::ItemCollapsed(ColumnTreeModel* model,
									   ColumnTreeItem* item)
{
}

// ItemsShown
void
ColumnTreeModelListener::ItemsShown(ColumnTreeModel* model, int32 index,
									int32 count)
{
}

// ItemsHidden
void
ColumnTreeModelListener::ItemsHidden(ColumnTreeModel* model, int32 index,
									 int32 count, bool before)
{
}

// ItemsSorted
void
ColumnTreeModelListener::ItemsSorted(ColumnTreeModel* model)
{
}

