// DefaultColumnTreeItemHandle.cpp

#include <algorithm>

#ifdef __HAIKU__
#	include <ControlLook.h>
#endif
#include <View.h>

#include "ColumnTreeItem.h"
#include "ColumnTreeModel.h"
#include "ColumnTreeViewColors.h"
#include "DefaultColumnTreeItemHandle.h"

// TODO: That's not nice. We assume how much e.g. the text is indented in
// text items.
static const int32 kInItemIndentation		= 8;
static const int32 kHandleTriangleWidth		= 11;
static const int32 kHandleTriangleHeight	= 7;
static const int32 kHandleTriangleHeightDiff1
	= (kHandleTriangleWidth - kHandleTriangleHeight) / 2;
static const int32 kHandleTriangleHeightDiff2
	= (kHandleTriangleWidth - kHandleTriangleHeight)
	  - kHandleTriangleHeightDiff1;
static const int32 kHandleTriangleInset		= 2;
static const int32 kHandleWidth				= kHandleTriangleWidth
											  + 2 * kHandleTriangleInset;
static const int32 kIndentationPerLevel		= kInItemIndentation
											  + kHandleWidth
											  - kHandleTriangleHeightDiff1;

// constructor
DefaultColumnTreeItemHandle::DefaultColumnTreeItemHandle()
	: ColumnTreeItemHandle()
{
}

// destructor
DefaultColumnTreeItemHandle::~DefaultColumnTreeItemHandle()
{
}

// GetIndentation
float
DefaultColumnTreeItemHandle::GetIndentation(int32 level)
{
	return kIndentationPerLevel * level;
}

// GetHandleRect
BRect
DefaultColumnTreeItemHandle::GetHandleRect(ColumnTreeItem* item, BRect frame)
{
	if (item && fModel && fModel->CountSubItems(item))
		return _GetHandleRect(frame, fModel->LevelOf(item)) & frame;
	return BRect();
}

// Draw
void
DefaultColumnTreeItemHandle::Draw(BView* view, ColumnTreeItem* item,
	Column* column, BRect frame, BRect updateRect, uint32 flags,
	const column_tree_item_colors* colors)
{
	if (item == NULL)
		return;

	// draw the background
	item->DrawBackground(view, column, frame, updateRect, item->Flags(),
		colors);

	// draw the handle, if any
	if (fModel == NULL || fModel->CountSubItems(item) == 0)
		return;

	BRect handleRect = _GetHandleRect(frame, fModel->LevelOf(item));

	if (be_control_look != NULL) {
		handleRect.InsetBy(1, 1);
		uint32 arrowDirection = item->IsExpanded()
			? BControlLook::B_UP_ARROW : BControlLook::B_DOWN_ARROW;
		float tint = (flags & COLUMN_TREE_ITEM_HANDLE_HOVER) != 0
			? B_DARKEN_4_TINT : B_DARKEN_2_TINT;
		const rgb_color& color = (flags & COLUMN_TREE_ITEM_SELECTED) != 0
			? colors->selected_background : colors->background;
		be_control_look->DrawArrowShape(view, handleRect, updateRect,
			color, arrowDirection, 0, tint);
	} else {
		int32 left = int32(handleRect.left + kHandleTriangleInset);
		int32 right = int32(handleRect.right - kHandleTriangleInset);
		int32 top = int32(handleRect.top + kHandleTriangleInset);
		int32 bottom = int32(handleRect.bottom - kHandleTriangleInset);
// TODO: Fix the colors.
		if (item->IsExpanded()) {
			top += kHandleTriangleHeightDiff1;
			bottom -= kHandleTriangleHeightDiff2;
			BPoint p1(left, top);
			BPoint p2(right, top);
			BPoint p3((left + right) / 2, bottom);
			view->SetHighColor(colors->shadow);
			view->FillTriangle(p1, p2, p3);
			view->SetHighColor(colors->foreground);
			view->StrokeTriangle(p1, p2, p3);
		} else {
			left += kHandleTriangleHeightDiff1;
			right -= kHandleTriangleHeightDiff2;
			BPoint p1(left, top);
			BPoint p2(right, (top + bottom) / 2);
			BPoint p3(left, bottom);
			view->SetHighColor(colors->shadow);
			view->FillTriangle(p1, p2, p3);
			view->SetHighColor(colors->foreground);
			view->StrokeTriangle(p1, p2, p3);
		}
	}
}

// _GetHandleRect
BRect
DefaultColumnTreeItemHandle::_GetHandleRect(BRect frame, int32 level)
{
	frame.left += GetIndentation(level - 1) + kInItemIndentation
				  - kHandleTriangleHeightDiff1;
	frame.right = frame.left + kHandleWidth - 1;
	int32 height = (int32)frame.bottom - (int32)frame.top + 1;
	int32 handleHeight = MIN((int32)height, (int32)kHandleWidth);
	frame.top += (height - handleHeight) / 2;
	frame.bottom = frame.top + kHandleWidth - 1;
	return frame;
}

