// ColumnTreeModelListener.h

#ifndef COLUMN_TREE_MODEL_LISTENER_H
#define COLUMN_TREE_MODEL_LISTENER_H

#include <SupportDefs.h>

class ColumnTreeItem;
class ColumnTreeModel;

class ColumnTreeModelListener {
public:
								ColumnTreeModelListener();
	virtual						~ColumnTreeModelListener();

	virtual	void				ItemsAdded(ColumnTreeModel* model,
										   ColumnTreeItem* super,
										   int32 index, int32 count);
	virtual	void				ItemsRemoved(ColumnTreeModel* model,
											 ColumnTreeItem* super,
											 int32 index, int32 count,
											 bool before);

	virtual	void				ItemExpanded(ColumnTreeModel* model,
											 ColumnTreeItem* item);
	virtual	void				ItemCollapsed(ColumnTreeModel* model,
											  ColumnTreeItem* item);

	virtual	void				ItemsShown(ColumnTreeModel* model,
										   int32 index, int32 count);
	virtual	void				ItemsHidden(ColumnTreeModel* model,
											int32 index, int32 count,
											bool before);

	virtual	void				ItemsSorted(ColumnTreeModel* model);
};

#endif	// COLUMN_TREE_MODEL_LISTENER_H
