// ColumnTreeItem.cpp

#include <View.h>

#include "ColumnTreeItem.h"
#include "Column.h"
#include "ColumnTreeView.h"
#include "ColumnTreeViewColors.h"

// constructor
ColumnTreeItem::ColumnTreeItem(float height)
	: fModelData(NULL),
	  fHeight(height),
	  fFlags(0),
	  fYOffset(0)
	  
{
}

// destructor
ColumnTreeItem::~ColumnTreeItem()
{
}

// Height
float
ColumnTreeItem::Height() const
{
	return fHeight;
}

// Draw
void
ColumnTreeItem::Draw(BView* view, Column* column, BRect frame,
					 BRect updateRect, uint32 flags,
					 const column_tree_item_colors* colors)
{
}

// DrawBackground
void
ColumnTreeItem::DrawBackground(BView* view, Column* column, BRect frame,
							   BRect rect, uint32 flags,
							   const column_tree_item_colors* colors)
{
	bool selected = (flags & COLUMN_TREE_ITEM_SELECTED);
	// set the colors according to the selection state
	if (selected)
		view->SetLowColor(colors->selected_background);
	else
		view->SetLowColor(colors->background);
	view->FillRect(rect, B_SOLID_LOW);
}


// AddFlags
void
ColumnTreeItem::AddFlags(uint32 flags)
{
	fFlags |= flags;
}

// ClearFlags
void
ColumnTreeItem::ClearFlags(uint32 flags)
{
	fFlags &= ~flags;
}

// SetFlags
void
ColumnTreeItem::SetFlags(uint32 flags)
{
	fFlags = flags;
}

// Flags
uint32
ColumnTreeItem::Flags() const
{
	return fFlags;
}

// SetSelected
void
ColumnTreeItem::SetSelected(bool selected)
{
	if (selected)
		AddFlags(COLUMN_TREE_ITEM_SELECTED);
	else
		ClearFlags(COLUMN_TREE_ITEM_SELECTED);
}

// IsSelected
bool
ColumnTreeItem::IsSelected() const
{
	return (fFlags & COLUMN_TREE_ITEM_SELECTED);
}

// SetVisible
void
ColumnTreeItem::SetVisible(bool visible)
{
	if (visible)
		AddFlags(COLUMN_TREE_ITEM_VISIBLE);
	else
		ClearFlags(COLUMN_TREE_ITEM_VISIBLE);
}

// IsVisible
bool
ColumnTreeItem::IsVisible() const
{
	return (fFlags & COLUMN_TREE_ITEM_VISIBLE);
}

// SetExpanded
void
ColumnTreeItem::SetExpanded(bool expanded)
{
	if (expanded)
		AddFlags(COLUMN_TREE_ITEM_EXPANDED);
	else
		ClearFlags(COLUMN_TREE_ITEM_EXPANDED);
}

// IsExpanded
bool
ColumnTreeItem::IsExpanded() const
{
	return (fFlags & COLUMN_TREE_ITEM_EXPANDED);
}

// SetYOffset
void
ColumnTreeItem::SetYOffset(float offset)
{
	fYOffset = offset;
}

// YOffset
float
ColumnTreeItem::YOffset() const
{
	return fYOffset;
}

// SetModelData
void
ColumnTreeItem::SetModelData(void* data)
{
	fModelData = data;
}

// GetModelData
void*
ColumnTreeItem::GetModelData() const
{
	return fModelData;
}

