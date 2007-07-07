// ColumnTreeItem.h

#ifndef COLUMN_TREE_ITEM_H
#define COLUMN_TREE_ITEM_H

#include <Rect.h>

// flags
enum {
	COLUMN_TREE_ITEM_USER_FLAGS	= 0x00,
	COLUMN_TREE_ITEM_SELECTED	= 0x01,
	COLUMN_TREE_ITEM_VISIBLE	= 0x02,
	COLUMN_TREE_ITEM_EXPANDED	= 0x04,
};

class BView;

class Column;
class ColumnTreeView;
struct column_tree_item_colors;

class ColumnTreeItem {
 public:
								ColumnTreeItem(float height);
	virtual						~ColumnTreeItem();

			void				SetHeight(float height);
			float				Height() const;

	virtual	void				Draw(BView* view, Column* column, BRect frame,
									 BRect updateRect, uint32 flags,
									 const column_tree_item_colors* colors);

			void				DrawBackground(BView* view, Column* column,
									BRect frame, BRect rect, uint32 flags,
									const column_tree_item_colors* colors);

	// service methods for the ColumnTreeView implementation
			void				AddFlags(uint32 flags);
			void				ClearFlags(uint32 flags);
			void				SetFlags(uint32 flags);
			uint32				Flags() const;

			void				SetSelected(bool selected);
			bool				IsSelected() const;

			void				SetVisible(bool visible);
			bool				IsVisible() const;

			void				SetExpanded(bool expanded);
			bool				IsExpanded() const;

			void				SetYOffset(float offset);
			float				YOffset() const;

	// service method for the model implementation
	// TODO: remove those and rather add a hash table to the models, that
	// need it
			void				SetModelData(void* data);
			void*				GetModelData() const;

 private:
			void*				fModelData;
			float				fHeight;
			uint32				fFlags;
			float				fYOffset;
};

#endif	// COLUMN_TREE_ITEM_H
