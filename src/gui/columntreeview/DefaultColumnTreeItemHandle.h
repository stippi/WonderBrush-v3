// DefaultColumnTreeItemHandle.h

#ifndef DEFAULT_COLUMN_TREE_ITEM_HANDLE_H
#define DEFAULT_COLUMN_TREE_ITEM_HANDLE_H

#include "ColumnTreeItemHandle.h"

class DefaultColumnTreeItemHandle : public ColumnTreeItemHandle {
public:
								DefaultColumnTreeItemHandle();
	virtual						~DefaultColumnTreeItemHandle();

	virtual	float				GetIndentation(int32 level);

	virtual	BRect				GetHandleRect(ColumnTreeItem* item,
											  BRect frame);

	virtual	void				Draw(BView* view, ColumnTreeItem* item,
									 Column* column, BRect frame,
									 BRect updateRect, uint32 flags,
									 const column_tree_item_colors* colors);

private:
			BRect				_GetHandleRect(BRect frame, int32 level);

			int32				fInItemIndentation;
			int32				fHandleTriangleWidth;
			int32				fHandleTriangleHeight;
			int32				fHandleTriangleHeightDiff1;
			int32				fHandleTriangleHeightDiff2;
			int32				fHandleTriangleInset;
			int32				fHandleWidth;
			int32				fIndentationPerLevel;
};

#endif	// DEFAULT_COLUMN_TREE_ITEM_HANDLE_H
