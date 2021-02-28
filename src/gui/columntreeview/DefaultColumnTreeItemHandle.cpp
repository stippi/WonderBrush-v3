// DefaultColumnTreeItemHandle.cpp

#include <algorithm>
#include <stdio.h>

#ifdef __HAIKU__
#	include <ControlLook.h>
#endif
#include <View.h>

#include "support_ui.h"

#include "ColumnTreeItem.h"
#include "ColumnTreeModel.h"
#include "ColumnTreeViewColors.h"
#include "DefaultColumnTreeItemHandle.h"

// constructor
DefaultColumnTreeItemHandle::DefaultColumnTreeItemHandle()
	: ColumnTreeItemHandle()
{
	int32 uiScale = ui_scale();
	fInItemIndentation = 8 * uiScale;
	fHandleTriangleWidth = 11 * uiScale;
	fHandleTriangleHeight = 7 * uiScale;
	fHandleTriangleHeightDiff1 = (fHandleTriangleWidth - fHandleTriangleHeight) / 2;
	fHandleTriangleHeightDiff2 = (fHandleTriangleWidth - fHandleTriangleHeight)
	  - fHandleTriangleHeightDiff1;
	fHandleTriangleInset = 2 * uiScale;
	fHandleWidth = fHandleTriangleWidth + 2 * fHandleTriangleInset;
	fIndentationPerLevel = fInItemIndentation + fHandleWidth
		- fHandleTriangleHeightDiff1;
}

// destructor
DefaultColumnTreeItemHandle::~DefaultColumnTreeItemHandle()
{
}

// GetIndentation
float
DefaultColumnTreeItemHandle::GetIndentation(int32 level)
{
	return fIndentationPerLevel * level;
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
			? BControlLook::B_DOWN_ARROW : BControlLook::B_RIGHT_ARROW;
		float tint = (flags & COLUMN_TREE_ITEM_HANDLE_HOVER) != 0
			? B_DARKEN_4_TINT : B_DARKEN_2_TINT;
		const rgb_color& color = (flags & COLUMN_TREE_ITEM_SELECTED) != 0
			? colors->selected_background : colors->background;
		be_control_look->DrawArrowShape(view, handleRect, updateRect,
			color, arrowDirection, 0, tint);
	} else {
		int32 left = int32(handleRect.left + fHandleTriangleInset);
		int32 right = int32(handleRect.right - fHandleTriangleInset);
		int32 top = int32(handleRect.top + fHandleTriangleInset);
		int32 bottom = int32(handleRect.bottom - fHandleTriangleInset);
// TODO: Fix the colors.
		if (item->IsExpanded()) {
			top += fHandleTriangleHeightDiff1;
			bottom -= fHandleTriangleHeightDiff2;
			BPoint p1(left, top);
			BPoint p2(right, top);
			BPoint p3((left + right) / 2, bottom);
			view->SetHighColor(colors->shadow);
			view->FillTriangle(p1, p2, p3);
			view->SetHighColor(colors->foreground);
			view->StrokeTriangle(p1, p2, p3);
		} else {
			left += fHandleTriangleHeightDiff1;
			right -= fHandleTriangleHeightDiff2;
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
	frame.left += GetIndentation(level - 1) + fInItemIndentation
				  - fHandleTriangleHeightDiff1;
	frame.right = frame.left + fHandleWidth - 1;
	int32 height = (int32)frame.bottom - (int32)frame.top + 1;
	int32 handleHeight = MIN(height, fHandleWidth);
	frame.top += (height - handleHeight) / 2;
	frame.bottom = frame.top + fHandleWidth - 1;
	return frame;
}

