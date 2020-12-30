// ColumnTreeView.cpp

//#include <algobase.h>
#include <new>
#include <stdio.h>

#include <Message.h>
#ifdef __HAIKU__
#	include <LayoutUtils.h>
#endif
#include <Window.h>

#include "ColumnTreeView.h"
#include "Column.h"
#include "ColumnHeaderView.h"
#include "ColumnTreeItem.h"
#include "ColumnTreeItemHandle.h"
#include "ColumnTreeItemCompare.h"
#include "ColumnTreeViewColors.h"
#include "DefaultColumnTreeItemHandle.h"
#include "ScrollView.h"
#include "Sort.h"

// only temporary
#include "ColumnListModel.h"

using namespace ColumnTreeViewStates;
using std::nothrow;

// ItemCompare

class ColumnTreeView::ItemCompare : public ColumnTreeItemCompare {
 public:
								ItemCompare();
								ItemCompare(
									column_tree_compare_function* cmpFunc,
			 						Column* primary, Column* secondary);

			void				SetTo(column_tree_compare_function* cmpFunc,
			 						  Column* primary, Column* secondary);

			bool				IsValid() const;

	virtual	bool				operator()(const ColumnTreeItem* item1,
										   const ColumnTreeItem* item2) const;

 private:
			column_tree_compare_function* fCmpFunc;
			Column*				fPrimary;
			Column*				fSecondary;
			int					fInverse1;
			int					fInverse2;
};

// constructor
ColumnTreeView::ItemCompare::ItemCompare()
	: fCmpFunc(NULL),
	  fPrimary(NULL),
	  fSecondary(NULL),
	  fInverse1(1),
	  fInverse2(1)
{
}

// constructor
ColumnTreeView::ItemCompare::ItemCompare(column_tree_compare_function* cmpFunc,
										 Column* primary, Column* secondary)
	: fCmpFunc(NULL),
	  fPrimary(NULL),
	  fSecondary(NULL),
	  fInverse1(1),
	  fInverse2(1)
{
	SetTo(cmpFunc, primary, secondary);
}

// SetTo
void
ColumnTreeView::ItemCompare::SetTo(column_tree_compare_function* cmpFunc,
								   Column* primary, Column* secondary)
{
	fCmpFunc = cmpFunc;
	fPrimary = primary;
	fSecondary = secondary;
	fInverse1 = 1;
	fInverse2 = 1;
	// make clear what key(s) to sort
	if (fSecondary && (fSecondary->Flags() & COLUMN_SORT_INVERSE))
		fInverse2 = -1;
	if (fPrimary) {
		if ((fPrimary->Flags() & COLUMN_SORT_INVERSE))
			fInverse1 = -1;
	} else {
		fPrimary = fSecondary;
		fInverse1 = fInverse2;
		fSecondary = NULL;
	}
}

// IsValid
bool
ColumnTreeView::ItemCompare::IsValid() const
{
	return (fCmpFunc && fPrimary);
}

// ()
bool
ColumnTreeView::ItemCompare::operator()(const ColumnTreeItem* item1,
										const ColumnTreeItem* item2) const
{
	int cmp = (*fCmpFunc)(item1, item2, fPrimary) * fInverse1;
	if (cmp == 0) {
		if (fSecondary)
			cmp = (*fCmpFunc)(item1, item2, fSecondary) * fInverse2;
	}
	return (cmp < 0);
}



// #pragma mark - ColumnTreeView

// constructor
ColumnTreeView::ColumnTreeView(BRect frame)
	:
	BView(frame, NULL, B_FOLLOW_NONE,
		B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE),
	BInvoker(new BMessage(B_SIMPLE_DATA), this),
	Scrollable(),	// DataRect == (0, 0, 0, 0), ScrollOffset = (0, 0)
	ColumnTreeModelListener(),
	fColumns(20),
	fVisibleColumns(20),
//	fItems(100),
	fSelectedItems(),
	fDefaultItemHeight(0.0f)
{
	_Init();
}

#ifdef __HAIKU__

// constructor
ColumnTreeView::ColumnTreeView()
	:
	BView(NULL, B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE),
	BInvoker(new BMessage(B_SIMPLE_DATA), this),
	Scrollable(),	// DataRect == (0, 0, 0, 0), ScrollOffset = (0, 0)
	ColumnTreeModelListener(),
	fColumns(20),
	fVisibleColumns(20),
//	fItems(100),
	fSelectedItems(),
	fDefaultItemHeight(0.0f)
{
	_Init();
}

#endif // __HAIKU__

// destructor
ColumnTreeView::~ColumnTreeView()
{
	_ChangeState(NULL);
	SetModel(NULL);
	delete fColors;
	delete fInvocationMessage;
	delete fSelectionMessage;
	// delete columns
	for (int32 i = 0; Column* column = _OrderedColumnAt(i); i++)
		delete column;
}

// AttachedToWindow
void
ColumnTreeView::AttachedToWindow()
{
	SetTarget(Window());
}

// MessageReceived
void
ColumnTreeView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_MOUSE_WHEEL_CHANGED:
		{
			BPoint delta;
			if (message->FindFloat("be:wheel_delta_x", &delta.x) != B_OK)
				delta.x = 0.0f;
			if (message->FindFloat("be:wheel_delta_y", &delta.y) != B_OK)
				delta.y = 0.0f;
			delta.x *= 10.0f;
			if (ColumnTreeItem* item = ItemAt(0))
				delta.y *= item->Height() + 1;
			else
				delta.y *= 10.0f;
			SetScrollOffset(ScrollOffset() + delta);
			break;
		}
		default:
			BView::MessageReceived(message);
			break;
	}
}

// Draw
void
ColumnTreeView::Draw(BRect updateRect)
{
	Draw(this, updateRect);
}

// Draw
void
ColumnTreeView::Draw(BView* view, BRect updateRect)
{
	if (view == NULL)
		return;

	BRect itemsRect(_ActualItemsRect());
	// let the items draw themselves
	BRect itemsUpdateRect(updateRect & itemsRect);
	if (itemsUpdateRect.IsValid()) {
		// some items have to be updated
		int32 first = IndexOf(itemsUpdateRect.top);
		int32 last = IndexOf(itemsUpdateRect.bottom);
		int32 firstColumn = _VisibleColumnIndexOf(itemsUpdateRect.left);
		int32 lastColumn = _VisibleColumnIndexOf(itemsUpdateRect.right);
		for (int32 i = first; i <= last; i++) {
			if (ColumnTreeItem* item = ItemAt(i))
				_DrawItem(item, view, firstColumn, lastColumn,
						  itemsUpdateRect);
		}
	}
	// draw the background
	if (updateRect.right > itemsRect.right) {
		BRect rect(updateRect);
		rect.left = itemsRect.right + 1.0f;
		if (rect.IsValid()) {
			view->SetHighColor(fColors->background);
			view->FillRect(rect);
		}
	}
	if (updateRect.bottom > itemsRect.bottom) {
		BRect rect(updateRect);
		rect.top = itemsRect.bottom + 1.0f;
		// don't draw more than necessary:
		rect.right = MIN(rect.right, itemsRect.right);
		if (rect.IsValid()) {
			view->SetHighColor(fColors->background);
			view->FillRect(rect);
		}
	}
	// Let the current state draw, if it wants to.
	fState->Draw(view, updateRect);
}

// FrameResized
void
ColumnTreeView::FrameResized(float width, float height)
{
	SetVisibleSize(width, height - _HeaderViewRect().Height() - 1.0f);
	_Layout();
	BRect itemsRect(_ItemsRect());
	SetVisibleSize(itemsRect.Width(), itemsRect.Height());
}

// KeyDown
void
ColumnTreeView::KeyDown(const char* bytes, int32 numBytes)
{
	int32 count = CountItems();
	int32 firstSelected = CurrentSelection(0);
	int32 lastSelected = CurrentSelection(fSelectedItems.CountItems() - 1);
	uint32 modifiers = 0;
	Window()->CurrentMessage()->FindInt32("modifiers", (int32 *)&modifiers);
	bool extend = (modifiers & B_SHIFT_KEY) || (modifiers & B_OPTION_KEY);
	switch (bytes[0]) {
		case B_DOWN_ARROW:
		{
			if (fSelectedItems.IsEmpty()) {
				if (count > 0) {
					Select(0, false);
					ScrollToItem(0);
				}
			} else {
				int32 index = lastSelected + 1;
				if (index >= count)
					index--;
				Select(index, extend);
				ScrollToItem(index);
			}
			break;
		}
		case B_UP_ARROW:
		{
			if (fSelectedItems.IsEmpty()) {
				if (count > 0) {
					Select(count - 1, false);
					ScrollToItem(count - 1);
				}
			} else {
				int32 index = firstSelected - 1;
				if (index < 0)
					index = 0;
				Select(index, extend);
				ScrollToItem(index);
			}
			break;
		}
		case B_PAGE_DOWN:
			SetScrollOffset(fCurrentScrollOffset
							+ BPoint(0, _ItemsRect().Height()));
			break;
		case B_PAGE_UP:
			SetScrollOffset(fCurrentScrollOffset
							- BPoint(0, _ItemsRect().Height()));
			break;
		case B_ENTER:
//		case B_RETURN:
			if (!fSelectedItems.IsEmpty())
				_InternalInvoke(fInvocationMessage);
			break;
		default:
			BView::KeyDown(bytes, numBytes);
			break;
	}
}

// MakeFocus
void
ColumnTreeView::MakeFocus(bool focusState)
{
	if (ScrollView* scrollView = dynamic_cast<ScrollView*>(ScrollSource())) {
		if (scrollView->Child() == this)
			scrollView->ChildFocusChanged(focusState);
	}
	BView::MakeFocus(focusState);
}

// MouseDown
void
ColumnTreeView::MouseDown(BPoint point)
{
	uint32 buttons = 0;
	uint32 modifiers = 0;
	int32 clicks = 1;
	Window()->CurrentMessage()->FindInt32("buttons", (int32 *)&buttons);
	Window()->CurrentMessage()->FindInt32("modifiers", (int32 *)&modifiers);
	Window()->CurrentMessage()->FindInt32("clicks", &clicks);
	// begin double click feature
	int32 index = IndexOf(point);
	bigtime_t clickTime = system_time();
	if (index == fLastClickedIndex
		&& clickTime - fLastClickTime <= fDoubleClickTime) {
		ItemDoubleClicked(fLastClickedIndex);
	}
	fLastClickedIndex = index;
	fLastClickTime = clickTime;
	// end double click feature
	fState->Pressed(point, buttons, modifiers, clicks);
}

// MouseMoved
void
ColumnTreeView::MouseMoved(BPoint point, uint32 transit,
							 const BMessage* message)
{
	switch (transit) {
		case B_ENTERED_VIEW:
			fState->Entered(point, message);
			break;
		case B_EXITED_VIEW:
			fState->Exited(point, message);
			break;
		default:
			fState->Moved(point, transit, message);
			break;
	}
}

// MouseUp
void
ColumnTreeView::MouseUp(BPoint point)
{
	uint32 buttons = 0;
	uint32 modifiers = 0;
	Window()->CurrentMessage()->FindInt32("buttons", (int32 *)&buttons);
	Window()->CurrentMessage()->FindInt32("modifiers", (int32 *)&modifiers);
	fState->Released(point, buttons, modifiers);
}

void
ColumnTreeView::GetPreferredSize(float* _width, float* _height)
{
	if (_width != NULL) {
		*_width = 100;
	}
	if (_height != NULL) {
		*_height = fHeaderView->Bounds().Height() + 50;
	}
}

#ifdef __HAIKU__

BSize
ColumnTreeView::MinSize()
{
	float width;
	float height;
	GetPreferredSize(&width, &height);
	return BLayoutUtils::ComposeSize(ExplicitMinSize(), BSize(width, height));
}

BSize
ColumnTreeView::MaxSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMaxSize(),
		BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));
}

#endif // __HAIKU__


// Invoke
/*status_t
ColumnTreeView::Invoke(BMessage* message)
{
	BMessage clone(InvokeKind());
	if (message)
		clone = *message;
	clone.AddInt64("when", system_time());
	clone.AddPointer("source", (void*)this);
	int32 count = fSelectedItems.CountItems();
	for (int32 i = 0; i < count; i++)
		clone.AddInt32("index", CurrentSelection(i));
	return BInvoker::Invoke(&clone);
}*/

// ScrollOffsetChanged
void
ColumnTreeView::ScrollOffsetChanged(BPoint oldOffset, BPoint newOffset)
{
	if (fScrollingEnabled) {
		fCurrentScrollOffset = newOffset;
		_Layout();
		BRect itemsRect(_ItemsRect());
		CopyBits(itemsRect.OffsetByCopy(newOffset - oldOffset), itemsRect);
	}
}

// SetModel
void
ColumnTreeView::SetModel(ColumnTreeModel* model)
{
	if (fModel)
		fModel->RemoveListener(this);
	fModel = model;
	if (fItemHandle)
		fItemHandle->SetModel(fModel);
	if (fModel)
		fModel->AddListener(this);
}

// GetModel
ColumnTreeModel*
ColumnTreeView::GetModel() const
{
	return fModel;
}

// AddColumn
void
ColumnTreeView::AddColumn(Column* column, int32 index)
{
	if (column && column->Header()) {
		int32 count = CountColumns();
		// check index
		if (index < 0 || index > count)
			index = count;
		_ReindexColumns(index, 1);
		// add column
		fColumns.AddItem((void*)column, index);
		column->SetParent(this);
		column->SetIndex(index);
		// visible?
		if (column->IsVisible())
			_AddVisibleColumn(column);
	}
}

// RemoveColumn
Column*
ColumnTreeView::RemoveColumn(int32 index)
{
	Column* column = NULL;
	int32 orderedIndex = OrderedColumnIndexOf(index);
	if (orderedIndex >= 0)
		column = _RemoveOrderedColumn(orderedIndex);
	return column;
}

// RemoveColumn
bool
ColumnTreeView::RemoveColumn(Column* column)
{
	int32 index = OrderedColumnIndexOf(column);
	if (index >= 0)
		return _RemoveOrderedColumn(index);
	return false;
}

// FindColumn
Column*
ColumnTreeView::FindColumn(const char* name) const
{
	Column* column = NULL;
	for (int32 i = 0; Column* candidate = (Column*)fColumns.ItemAt(i); i++) {
		if (strcmp(candidate->Name(), name) == 0) {
			column = candidate;
			break;
		}
	}
	return column;
}

// ColumnAt
Column*
ColumnTreeView::ColumnAt(int32 index) const
{
	return _OrderedColumnAt(OrderedColumnIndexOf(index));
}

// ColumnIndexOf
int32
ColumnTreeView::ColumnIndexOf(Column* column) const
{
	if (fColumns.HasItem((void*)column))
		return column->Index();
	return -1;
}

// CountColumns
int32
ColumnTreeView::CountColumns() const
{
	return fColumns.CountItems();
}

// HideColumn
void
ColumnTreeView::HideColumn(int32 index)
{
	_HideOrderedColumn(OrderedColumnIndexOf(index));
}

// HideColumn
void
ColumnTreeView::HideColumn(Column* column)
{
	if (fColumns.HasItem((void*)column))
		_HideOrderedColumn(column);
}

// ShowColumn
void
ColumnTreeView::ShowColumn(int32 index)
{
	_ShowOrderedColumn(OrderedColumnIndexOf(index));
}

// ShowColumn
void
ColumnTreeView::ShowColumn(Column* column)
{
	if (fColumns.HasItem((void*)column))
		_ShowOrderedColumn(column);
}

// InvalidateHeader
void
ColumnTreeView::InvalidateHeader(int32 index)
{
	fHeaderView->_InvalidateHeaders(index, -1);
}

// AddItem
/*!	\brief Inserts an item at the top level of the tree.

	\a index specifies a position at the top level of the tree. After
	successful insertion, \c SubItemAt(NULL, index) returns \a item.

	\param item The item to be inserted.
	\param index The index at which to add the item.
	\return \c true, if everything went fine, \c false otherwise.
*/
void
ColumnTreeView::AddItem(ColumnTreeItem* item, int32 index)
{
	if (!fModel || !item)
		return /*false*/;
	/*return*/ fModel->AddSubItem(NULL, item, index);
}

// RemoveItem
/*!	\brief Removes the item at a specified index at the top level of the tree.
	\param index The index of the item at the top level to be removed.
	\return The removed item, if everything went fine, \c NULL otherwise
			(e.g. if the index is out of range).
*/
ColumnTreeItem*
ColumnTreeView::RemoveItem(int32 index)
{
// TODO: Rather make remove the item at the visible index!
	if (!fModel)
		return NULL;
	return fModel->RemoveSubItem(NULL, index);
}

// RemoveItem
/*!	\brief Removes the specified item from the tree.
	\param item The item to be removed.
	\return \c true, if everything went fine, \c false otherwise.
*/
bool
ColumnTreeView::RemoveItem(ColumnTreeItem* item)
{
	if (!fModel)
		return false;
	return fModel->RemoveItem(item);
}

// RemoveItems
/*!	\brief Removes the specified number of items at a given index at the top
		   level of the tree.
	\param index The index of the first item at the top level to be removed.
	\param count The number of items to be removed.
	\return \c true, if everything went fine, \c false otherwise
			(e.g. if the specified range is out of range).
*/
bool
ColumnTreeView::RemoveItems(int32 index, int32 count)
{
// TODO: Rather make remove the items at the visible index!
	if (!fModel)
		return false;
	return fModel->RemoveSubItems(NULL, index, count);
}

// HasItem
/*!	\brief Returns whether the tree contains the given item.
	\param item The item.
	\return \c true, if the tree contains the item, \c false otherwise.
*/
bool
ColumnTreeView::HasItem(ColumnTreeItem* item) const
{
	if (!fModel)
		return false;
	return fModel->HasItem(item);
}

// ItemAt
/*!	\brief Returns the item at the specified visible index.
	\param The item's index into the list of visible items.
	\return The item at the specified visible index, or \c NULL, if the
			index is out of range.
*/
ColumnTreeItem*
ColumnTreeView::ItemAt(int32 index) const
{
	if (!fModel)
		return NULL;
	return fModel->VisibleItemAt(index);
}

// IndexOf
/*!	\brief Returns an item's index in the list of visible items.
	\param The item.
	\return The visible index of the item or \c -1 if the \a item is \c NULL,
			or not contained in the tree, or currently not visible.
*/
int32
ColumnTreeView::IndexOf(ColumnTreeItem* item) const
{
	if (!fModel)
		return -1;
	return fModel->VisibleIndexOf(item);
}

/*!	\brief Returns the visible index of the item that contains the supplied
		   point.
	\param point The point for which the item is sought (measured in view
		   coordinates).
	\return The visible index of the item at the given graphics coordinate,
			or \c -1 if there is no item at that position.
*/
int32
ColumnTreeView::IndexOf(BPoint point) const
{
	if (DataRect().OffsetBySelf(_ItemsRect().LeftTop() - fCurrentScrollOffset)
		.Contains(point)) {
		return IndexOf(point.y);
	}
	return -1;
}

// IndexOf
/*!	\brief Returns the visible index of the item that contains the supplied
		   Y coordinate.
	\param y The Y coordinate for which the item is sought (measured in view
		   coordinates).
	\return The visible index of the item at the given graphics Y coordinate,
			or \c -1 if there is no item at that position.
*/
int32
ColumnTreeView::IndexOf(float y) const
{
	// binary search
	int32 lower = 0;
	int32 upper = CountItems() - 1;
	if (upper >= 0 && _ItemPosition(lower).y <= y &&
		_ItemFrame(upper).bottom >= y) {
		while (lower < upper) {
			int32 mid = (lower + upper + 1) / 2;
			if (_ItemPosition(mid).y <= y)
				lower = mid;
			else
				upper = mid - 1;
		}
		return lower;
	}
	return -1;
}

// CountItems
/*!	\brief Returns the number of visible items.
	\return The number of visible items.
*/
int32
ColumnTreeView::CountItems() const
{
	if (!fModel)
		return 0;
	return fModel->CountVisibleItems();
}

// IsEmpty
/*!	Returns whether the tree is empty.
	\return \c true, if the tree is emptry, \c false otherwise.
*/
bool
ColumnTreeView::IsEmpty() const
{
	return (CountItems() == 0);
}

// IndentationOf
/*!	Returns the indentation of the given item.
*/
float
ColumnTreeView::IndentationOf(ColumnTreeItem* item) const
{
	if (item && fModel)
		return IndentationOf(fModel->LevelOf(item));
	return 0.0f;
}

// IndentationOf
/*!	Returns the indentation of the given level.
*/
float
ColumnTreeView::IndentationOf(int32 level) const
{
	if (fItemHandle)
		return fItemHandle->GetIndentation(level);
	return 0.0f;
}

// MakeEmpty
/*!	\brief Removes all (currently visible and invisible) items from the tree.
	\return \c true, if everything went fine, \c false otherwise (e.g. if the
			current model is read-only).
*/
void
ColumnTreeView::MakeEmpty()
{
	if (!fModel)
		return /*false*/;
	/*return*/ fModel->MakeEmpty();
}

// AddSubItem
/*!	\brief Inserts a subitem.

	The specified item is inserted at the end of the subitems of a given
	superitem.

	\param super The superitem under which \a item shall be inserted. May be
		   \c NULL, in which case the item is added at the top level.
	\param item The item to be inserted.
	\return \c true, if everything went fine, \c false otherwise.
*/
bool
ColumnTreeView::AddSubItem(ColumnTreeItem* super, ColumnTreeItem* item)
{
	if (!fModel)
		return false;
	return fModel->AddSubItem(super, item);
}

// AddSubItem
/*!	\brief Inserts a subitem at a given index.

	The specified item is inserted at the given index of the list subitems
	of a superitem.

	\param super The superitem under which \a item shall be inserted. May be
		   \c NULL, in which case the item is added at the top level.
	\param item The item to be inserted.
	\param index The index at which to insert the item.
	\return \c true, if everything went fine, \c false otherwise.
*/
bool
ColumnTreeView::AddSubItem(ColumnTreeItem* super, ColumnTreeItem* item,
						   int32 index)
{
	if (!fModel)
		return false;
	return fModel->AddSubItem(super, item, index);
}

// RemoveSubItem
/*!	\brief Removes a subitem at a specified index.
	\param super The superitem from which a subitem shall be removed. May be
		   \c NULL, in which case the item is removed at the top level of the
		   tree.
	\param index The index of the subitem to be removed.
	\return The removed item, or \c NULL if something went wrong.
*/
ColumnTreeItem*
ColumnTreeView::RemoveSubItem(ColumnTreeItem* super, int32 index)
{
	if (!fModel)
		return NULL;
	return fModel->RemoveSubItem(super, index);
}

// RemoveSubItems
/*!	\brief Removes a certain number of subitem at a specified index.
	\param super The superitem from which the subitems shall be removed.
		   May be \c NULL, in which case the items are removed at the top
		   level of the tree.
	\param index The index of the first subitem to be removed.
	\param count The number of items to be removed.
	\return The removed item, or \c NULL if something went wrong.
*/
bool
ColumnTreeView::RemoveSubItems(ColumnTreeItem* super, int32 index, int32 count)
{
	if (!fModel)
		return false;
	return fModel->RemoveSubItems(super, index, count);
}

// CountSubItems
/*!	\brief Returns the number of subitems of a given superitem.
	\param super The superitem. If \c NULL, the number of items at the topmost
		   level of the tree is returned.
	\return The number of subitems of the given superitem.
*/
int32
ColumnTreeView::CountSubItems(ColumnTreeItem* super)
{
	if (!fModel)
		return 0;
	return fModel->CountSubItems(super);
}

// CountSubItemsRecursive
/*!	\brief Returns the number of all items below a given superitem.
	\param super The superitem.
	\return The number of items below the given superitem.
*/
int32
ColumnTreeView::CountSubItemsRecursive(ColumnTreeItem* super, bool visible)
{
	if (!fModel || !super)
		return 0;
	if (visible && !super->IsExpanded())
		return 0;
	int32 subCount = fModel->CountSubItems(super);
	int32 count = subCount;
	for (int32 i = 0; i < subCount; i++) {
		ColumnTreeItem* item = fModel->SubItemAt(super, i);
		if (item != NULL)
			count += CountSubItemsRecursive(item, visible);
	}
	return count;
}

// SubItemAt
/*!	\brief Returns the subitem of a superitem at a specified index.
	\param super The superitem. If \c NULL, the index refers to the topmost
		   level of the tree.
	\return The subitem at the specified index, or \c NULL, if something
			went wrong.
*/
ColumnTreeItem*
ColumnTreeView::SubItemAt(ColumnTreeItem* super, int32 index)
{
	if (!fModel)
		return NULL;
	return fModel->SubItemAt(super, index);
}

// SubItemIndexOf
/*!	\brief Returns the index of a given subitem.

	The returned index is relative to the item's superitem. If the item lives
	at the tree's top level, the index at that level is returned.

	\param item The subitem.
	\return The index of the subitem, or \c -1, if the item is not contained
			in the tree.
*/
int32
ColumnTreeView::SubItemIndexOf(ColumnTreeItem* item)
{
	if (!fModel || !item)
		return -1;
	return fModel->SubItemIndexOf(item);
}

// SuperItemOf
/*!	\brief Returns the superitem of a given item.
	\param item The item whose superitem shall be returned.
	\return The superitem of the item. \c NULL, if the item lives at top
			level or is not contained in the tree.
*/
ColumnTreeItem*
ColumnTreeView::SuperItemOf(ColumnTreeItem* item)
{
	if (!fModel)
		return NULL;
	return fModel->SuperItemOf(item);
}

// LevelOf
/*!	\brief Returns the level of a given item.
	\param item The item whose level shall be returned.
	\return The level of the specified item. \c 0, if the model is a list-like
			model and the item lives at the top level, or if the item is not
			contained in the tree.
*/
int32
ColumnTreeView::LevelOf(ColumnTreeItem* item)
{
	if (!fModel)
		return 0;
	return fModel->LevelOf(item);
}

// InvalidateItem
void
ColumnTreeView::InvalidateItem(int32 index)
{
	if (ColumnTreeItem* item = ItemAt(index))
		Invalidate(_ItemFrame(item));
}

// InvalidateItem
void
ColumnTreeView::InvalidateItem(ColumnTreeItem* item)
{
	if (item)
		Invalidate(_ItemFrame(item));
}

// ItemChanged
//
// To be called, when an item has changed. The sort position is checked
// and the item is invalidated.
void
ColumnTreeView::ItemChanged(int32 index)
{
	ItemChanged(ItemAt(index));
}

// ItemChanged
//
// To be called, when an item has changed. The sort position is checked
// and the item is invalidated.
void
ColumnTreeView::ItemChanged(ColumnTreeItem* item)
{
	if (!item)
		return;
	ColumnTreeItem* super = SuperItemOf(item);
	int32 index = IndexOf(item);
	bool selected = item->IsSelected();
	RemoveSubItem(super, index);
	AddSubItem(super, item, index);
	if (selected)
		Select(IndexOf(item));
// TODO: Improve that.
//	fModel->ItemChanged(item);
	InvalidateItem(item);
}

// ScrollToItem
void
ColumnTreeView::ScrollToItem(int32 index)
{
	if (ColumnTreeItem* item = ItemAt(index)) {
		BRect itemFrame(_ItemFrame(item));
		BRect itemsRect(_ItemsRect());
		if (itemFrame.top < itemsRect.top) {
			SetScrollOffset(fCurrentScrollOffset +
							BPoint(0, itemFrame.top - itemsRect.top));
		} else if (itemFrame.bottom > itemsRect.bottom) {
			SetScrollOffset(fCurrentScrollOffset +
							BPoint(0, itemFrame.bottom - itemsRect.bottom));
		}
	}
}

// InitiateDrag
bool
ColumnTreeView::InitiateDrag(BPoint point, int32 index, bool wasSelected,
	BMessage* _message)
{
	return false;
}

// GetDropInfo
bool
ColumnTreeView::GetDropInfo(BPoint point, const BMessage& dragMessage,
	ColumnTreeItem** _super, int32* _subItemIndex, int32* _itemIndex)
{
	int32 index = IndexOf(point);
	ColumnTreeItem* item = ItemAt(index);
	int32 subIndex = SubItemIndexOf(item);

	BRect frame;
	if (item != NULL) {
		frame = ItemFrame(index);
	 	if (point.y >= (frame.top + frame.bottom) / 2) {
			// insertion after item
			index += 1;
			subIndex += 1;
			// If the item already has sub-items (and is expanded), we can't
			// insert an item before the first sub-item that is at the same
			// level as the item. To still be able to insert an item after the
			// hovered item, account for all sub items.
			if (item->IsExpanded())
				index += CountSubItemsRecursive(item, true);
		}
	} else if (Bounds().Contains(point)) {
		index = CountItems();
		subIndex = CountSubItems(NULL);
	}

	if (_super != NULL)
		*_super = SuperItemOf(item);

	if (_subItemIndex != NULL)
		*_subItemIndex = subIndex;

	if (_itemIndex != NULL)
		*_itemIndex = index;

	return true;
}

// HandleDrop
void
ColumnTreeView::HandleDrop(const BMessage& dragMessage, ColumnTreeItem* super,
	int32 subItemIndex, int32 itemIndex)
{
}

// ItemDoubleClicked
void
ColumnTreeView::ItemDoubleClicked(int32 index)
{
}

// ItemSelectedTwice
void
ColumnTreeView::ItemSelectedTwice(int32 index)
{
}

// ExpandItem
bool
ColumnTreeView::ExpandItem(ColumnTreeItem* item)
{
	if (item && fModel)
		return fModel->ExpandItem(item);
	return false;
}

// ExpandItem
bool
ColumnTreeView::ExpandItem(int32 index)
{
	if (fModel)
		return fModel->ExpandItem(ItemAt(index));
	return false;
}

// CollapseItem
bool
ColumnTreeView::CollapseItem(ColumnTreeItem* item)
{
	if (item && fModel)
		return fModel->CollapseItem(item);
	return false;
}

// CollapseItem
bool
ColumnTreeView::CollapseItem(int32 index)
{
	if (fModel)
		return fModel->CollapseItem(ItemAt(index));
	return false;
}

// SetSelectionMode
void
ColumnTreeView::SetSelectionMode(selection_mode mode)
{
	fSelectionMode = mode;
}

// CurrentSelection
int32
ColumnTreeView::CurrentSelection(int32 index) const
{
	return fSelectedItems.ItemAt(index) - 1;
}

// Deselect
void
ColumnTreeView::Deselect(int32 index)
{
	ColumnTreeItem* item = ItemAt(index);
	if (item && item->IsSelected()) {
		item->SetSelected(false);
		fSelectedItems.Remove(_FindSelectedItem(index));
		InvalidateItem(index);
		_InternalSelectionChanged();
	}
}

// DeselectAll
void
ColumnTreeView::DeselectAll()
{
	if (!fSelectedItems.IsEmpty()) {
		for (int32 i = 0;
			 ColumnTreeItem* item = ItemAt(CurrentSelection(i));
			 i++) {
			item->SetSelected(false);
		}
		fSelectedItems.Clear();
		Invalidate();
		_InternalSelectionChanged();
	}
}

// DeselectExcept
void
ColumnTreeView::DeselectExcept(int32 start, int32 finish)
{
	bool changed = false;
	if (!fSelectedItems.IsEmpty()) {
		for (int32 i = fSelectedItems.CountItems() - 1; i >= 0; i--) {
			int32 index = CurrentSelection(i);
			if (index < start || index > finish) {
				ColumnTreeItem* item = ItemAt(index);
				item->SetSelected(false);
				fSelectedItems.Remove(i);
				changed = true;
			}
		}
		if (changed) {
			Invalidate();
			_InternalSelectionChanged();
		}
	}
}

// IsItemSelected
bool
ColumnTreeView::IsItemSelected(int32 index) const
{
	if (ColumnTreeItem* item = ItemAt(index))
		return item->IsSelected();
	return false;
}

// ScrollToSelection
void
ColumnTreeView::ScrollToSelection()
{
	ScrollToItem(CurrentSelection(0));
}

// Select
void
ColumnTreeView::Select(int32 index, bool extend)
{
	if (ColumnTreeItem* item = ItemAt(index)) {
		if (!extend) {
			fSelectionDepth++;
			DeselectAll();
			fSelectionDepth--;
		}
		if (!item->IsSelected()) {
			int32 insertionIndex = _FindSelectionInsertionIndex(index);
			fSelectedItems.Add(index + 1, insertionIndex);
			item->SetSelected(true);
			InvalidateItem(item);
			_InternalSelectionChanged();
		}
	}
}

// Select
void
ColumnTreeView::Select(int32 start, int32 finish, bool extend)
{
	// check for valid indices first
	int32 count = CountItems();
	if (start >= 0 && start < count && finish >= start && finish < count) {
		if (!extend) {
			fSelectionDepth++;
			DeselectAll();
		}
		int32 insertionIndex = _FindSelectionInsertionIndex(start);
		for (int32 i = start; i <= finish; i++) {
			ColumnTreeItem* item = ItemAt(i);
			if (!item->IsSelected()) {
				fSelectedItems.Add(i + 1, insertionIndex);
				item->SetSelected(true);
			}
			insertionIndex++;
		}
		Invalidate();
		_InternalSelectionChanged();
	}
}

// SelectionChanged
void
ColumnTreeView::SelectionChanged()
{
}

// CountSelectedItems
int32
ColumnTreeView::CountSelectedItems() const
{
	return fSelectedItems.CountItems();
}

// SetInvocationMessage
void
ColumnTreeView::SetInvocationMessage(BMessage* message)
{
	delete fInvocationMessage;
	fInvocationMessage = message;

}

// InvocationMessage
BMessage*
ColumnTreeView::InvocationMessage() const
{
	return fInvocationMessage;
}

// InvocationCommand
uint32
ColumnTreeView::InvocationCommand() const
{
	if (fInvocationMessage)
		return fInvocationMessage->what;
	return 0;
}

// SetSelectionMessage
void
ColumnTreeView::SetSelectionMessage(BMessage* message)
{
	delete fSelectionMessage;
	fSelectionMessage = message;
}

// SelectionMessage
BMessage*
ColumnTreeView::SelectionMessage() const
{
	return fSelectionMessage;
}

// SelectionCommand
uint32
ColumnTreeView::SelectionCommand() const
{
	if (fSelectionMessage)
		return fSelectionMessage->what;
	return 0;
}

// ResizeColumn
void
ColumnTreeView::ResizeColumn(int32 index, float width)
{
	ResizeColumn(ColumnAt(index), width);
}

// ResizeColumn
void
ColumnTreeView::ResizeColumn(Column* column, float width)
{
	if (column) {
		if (column->IsVisible())
			ResizeVisibleColumn(_VisibleColumnIndexOf(column), width);
		else
			column->SetWidth(width);
	}
}

// MoveColumn
void
ColumnTreeView::MoveColumn(int32 index, int32 dest)
{
	MoveColumn(ColumnAt(index), dest);
}

// MoveColumn
void
ColumnTreeView::MoveColumn(Column* column, int32 dest)
{
	if (column) {
		if (column->IsVisible())
			MoveVisibleColumn(_VisibleColumnIndexOf(column), dest);
//		else
	}
}

// PrimarySortColumn
status_t
ColumnTreeView::PrimarySortColumn(int32 *outIndex,  bool *outInverse) const
{
	status_t ret = B_BAD_VALUE;
	if (outIndex && outInverse) {
		ret = B_ERROR;
		if (fPrimarySortColumn) {
			*outIndex = ColumnIndexOf(fPrimarySortColumn);
			*outInverse = fPrimarySortColumn->Flags() & COLUMN_SORT_INVERSE;
			ret = B_OK;
		} else {
			*outIndex = -1;
			*outInverse = true;
		}
	}
	return ret;
}

// PrimarySortColumn
status_t
ColumnTreeView::SecondarySortColumn(int32 *outIndex,  bool *outInverse) const
{
	status_t ret = B_BAD_VALUE;
	if (outIndex && outInverse) {
		ret = B_ERROR;
		if (fSecondarySortColumn) {
			*outIndex = ColumnIndexOf(fSecondarySortColumn);
			*outInverse = fSecondarySortColumn->Flags() & COLUMN_SORT_INVERSE;
			ret = B_OK;
		} else {
			*outIndex = -1;
			*outInverse = true;
		}
	}
	return ret;
}

// SetPrimarySortColumn
void
ColumnTreeView::SetPrimarySortColumn(int32 index, bool inverse)
{
	_SetPrimarySortOrderedColumn(OrderedColumnIndexOf(index), inverse);
}

// SetSecondarySortColumn
void
ColumnTreeView::SetSecondarySortColumn(int32 index, bool inverse)
{
	_SetSecondarySortOrderedColumn(OrderedColumnIndexOf(index), inverse);
}

// SetSortCompareFunction
void
ColumnTreeView::SetSortCompareFunction(column_tree_compare_function* function)
{
	fCompareFunction = function;
}

// Sort
void
ColumnTreeView::Sort()
{
	_UpdateItemCompare(true);
}

// SetColors
void
ColumnTreeView::SetColors(const column_tree_view_colors* colors)
{
	*fColors = *colors;
	fHeaderView->SetColors(&fColors->header_view_colors);
	Invalidate();
}

// Colors
const column_tree_view_colors*
ColumnTreeView::Colors() const
{
	return fColors;
}

// ResizeVisibleColumn
void
ColumnTreeView::ResizeVisibleColumn(int32 index, float width)
{
	if (Column* column = _VisibleColumnAt(index)) {
		float oldWidth = column->Width();
		column->SetWidth(width);
		// We don't need to do anything if nothing has changed.
		if (column->Width() != oldWidth) {
			_UpdateColumnXOffsets(index + 1, false);
			_InvalidateVisibleColumns(index, -1);
			fHeaderView->ResizeHeader(index, column->Width());
		}
	}
}

// MoveVisibleColumn
//
// Move the column at index /oldIndex/ so that it afterwards is at index
// /newIndex/. Both indices must be valid visible indices, otherwise no column
// is moved at all.
void
ColumnTreeView::MoveVisibleColumn(int32 oldIndex, int32 newIndex)
{
/*	int32 count = _CountVisibleColumns();
	// Check if the indices are valid and different.
	if (oldIndex >= 0 && oldIndex < count &&
		newIndex >= 0 && newIndex < count &&
		newIndex != oldIndex) {
		Column* column = _VisibleColumnAt(oldIndex);
		// move within the visible list
		fVisibleColumns.RemoveItem(oldIndex);
		fVisibleColumns.AddItem((void*)column, newIndex);
		// move within the column list
		fColumns.RemoveItem((void*)column);
		if (newIndex > 0) {
			fColumns.AddItem((void*)column,
				fColumns.IndexOf(fVisibleColumns.ItemAt(newIndex - 1)) + 1);
		} else
			fColumns.AddItem((void*)column, 0);
		// Update the graphics stuff.
		int32 first = min(oldIndex, newIndex);
//		int32 last = max(oldIndex, newIndex);
		_UpdateColumnXOffsets(first, true);
		// move header
		fHeaderView->MoveHeader(oldIndex, newIndex);
	}
*/
	MoveVisibleColumns(oldIndex, newIndex, 1);
}

// MoveVisibleColumns
//
// Move the /count/ columns at index /index/ so that they afterwards are at
// index /dest/. All indices must be valid visible indices, otherwise no
// column is moved at all.
void
ColumnTreeView::MoveVisibleColumns(int32 index, int32 dest, int32 count)
{
	int32 columnCount = _CountVisibleColumns();
	if (index >= 0 && count > 0 && index + count <= columnCount &&
		dest >= 0 && dest + count <= columnCount && index != dest) {
		// store columns in a list
		BList columns;
		for (int32 i = index; i < index + count; i++)
			columns.AddItem((void*)_VisibleColumnAt(i));
		// move within the visible list
		fVisibleColumns.RemoveItems(index, count);
		fVisibleColumns.AddList(&columns, dest);
		// move within the column list
		for (int32 i = 0; Column* column = (Column*)columns.ItemAt(i); i++)
			fColumns.RemoveItem((void*)column);
		if (dest > 0) {
			fColumns.AddList(&columns,
				fColumns.IndexOf(fVisibleColumns.ItemAt(dest - 1)) + 1);
		} else
			fColumns.AddList(&columns, 0);
		// Update the graphics stuff.
		int32 first = MIN(index, dest);
//		int32 last = MAX(index, dest) + count - 1;
		_UpdateColumnXOffsets(first, true);
		// move header
		fHeaderView->MoveHeaders(index, dest, count);
	}
}

// DisableScrolling
void
ColumnTreeView::DisableScrolling()
{
	fScrollingEnabled = false;
}

// EnableScrolling
void
ColumnTreeView::EnableScrolling()
{
	if (!fScrollingEnabled) {
		fScrollingEnabled = true;
		if (fCurrentScrollOffset != ScrollOffset()) {
//			fCurrentScrollOffset = ScrollOffset();
//			_Layout();
			ScrollOffsetChanged(fCurrentScrollOffset, ScrollOffset());
		}
	}
}

// SetPrimarySortVisibleColumn
void
ColumnTreeView::SetPrimarySortVisibleColumn(int32 index, bool inverse)
{
	_SetPrimarySortOrderedColumn(
		OrderedColumnIndexOf(_VisibleColumnAt(index)), inverse);
}

// SetSecondarySortVisibleColumn
void
ColumnTreeView::SetSecondarySortVisibleColumn(int32 index, bool inverse)
{
	_SetSecondarySortOrderedColumn(
		OrderedColumnIndexOf(_VisibleColumnAt(index)), inverse);
}

// DefaultItemHeight
float
ColumnTreeView::DefaultItemHeight()
{
	if (fDefaultItemHeight == 0.0f) {
		BFont font;
		GetFont(&font);
		fDefaultItemHeight = ceilf(font.Size() * 1.5);
	}
	return fDefaultItemHeight;
}

// ItemsAdded
void
ColumnTreeView::ItemsAdded(ColumnTreeModel* model, ColumnTreeItem* super,
						   int32 index, int32 count)
{
	if (model->CountSubItems(super) == count)
		InvalidateItem(super);
}

// ItemsRemoved
void
ColumnTreeView::ItemsRemoved(ColumnTreeModel* model, ColumnTreeItem* super,
							 int32 index, int32 count, bool before)
{
	if (!before && count > 0 && model->CountSubItems(super) == 0)
		InvalidateItem(super);
}

// ItemExpanded
void
ColumnTreeView::ItemExpanded(ColumnTreeModel* model, ColumnTreeItem* item)
{
//printf("ColumnTreeView::ItemExpanded(%p)\n", item);
	_InvalidateItems(fModel->VisibleIndexOf(item), 1);
}

// ItemCollapsed
void
ColumnTreeView::ItemCollapsed(ColumnTreeModel* model, ColumnTreeItem* item)
{
//printf("ColumnTreeView::ItemCollapsed(%p)\n", item);
	_InvalidateItems(fModel->VisibleIndexOf(item), 1);
}

// ItemsShown
void
ColumnTreeView::ItemsShown(ColumnTreeModel* model, int32 index, int32 count)
{
//printf("ColumnTreeView::ItemsShown(%ld, %ld)\n", index, count);
	// keep the selection in sync
	_ReindexSelectedItems(index, count);
	_UpdateItemYOffsets(index, false);
	_InvalidateItems(index, -1);
}

// ItemsHidden
void
ColumnTreeView::ItemsHidden(ColumnTreeModel* model, int32 index, int32 count,
							bool before)
{
//printf("ColumnTreeView::ItemsHidden(%ld, %ld, %d)\n", index, count, before);
	if (before) {
		// keep the selection in sync
		int32 first = _FindSelectionInsertionIndex(index);
		int32 last = _FindSelectionInsertionIndex(index + count);
		if (first < last) {
			for (int32 i = first; i < last; i++) {
				if (ColumnTreeItem* item = ItemAt(CurrentSelection(i)))
					item->SetSelected(false);
			}
			fSelectedItems.Remove(first, last - first);
			_InternalSelectionChanged();
		}
	} else {
		_ReindexSelectedItems(index + count, -count);
		_UpdateItemYOffsets(index, false);
		_InvalidateItems(index, -1);
	}
}

// ItemsSorted
void
ColumnTreeView::ItemsSorted(ColumnTreeModel* model)
{
//printf("ColumnTreeView::ItemsSorted()\n");
	_RebuildSelectionList();
	_UpdateItemYOffsets(0, false);
	Invalidate();
}

// _Init
void
ColumnTreeView::_Init()
{
	fModel = NULL;
	fItemHandle = new(nothrow) DefaultColumnTreeItemHandle;
	fCurrentScrollOffset = B_ORIGIN;
	fScrollingEnabled = true;
	fPrimarySortColumn = NULL;
	fSecondarySortColumn = NULL;
	fInvocationMessage = NULL;
	fSelectionMessage = NULL;
	fSelectionDepth = 0;
	fState = new(nothrow) OutsideState(this);
	fColors
		= new(nothrow) column_tree_view_colors(kDefaultColumnTreeViewColors);
	fCompareFunction = NULL;
	fItemCompare = NULL;
	fSelectionMode = CLV_MULTIPLE_SELECTION;
	fLastClickedIndex = -1;
	fLastClickTime = system_time();
	fDoubleClickTime = 500000;

	fHeaderView = new ColumnHeaderView();
	fHeaderView->SetParentView(this);
	fHeaderView->SetColors(&fColors->header_view_colors);
	AddChild(fHeaderView);
	_Layout();
	SetViewColor(B_TRANSPARENT_32_BIT);
	if (get_click_speed(&fDoubleClickTime) != B_OK)
		fDoubleClickTime = 500000;
	// set default model
	// TODO: replace with DefaultColumnTreeModel
	SetModel(new ColumnListModel);
	fItemCompare = new ItemCompare;
}

// _RemoveOrderedColumn
Column*
ColumnTreeView::_RemoveOrderedColumn(int32 index)
{
	Column* column = NULL;
	int32 count = CountColumns();
	if (index >= 0 && index < count) {
		column = (Column*)fColumns.RemoveItem(index);
		_ReindexColumns(column->Index(), -1);
		if (column->IsVisible())
			_RemoveVisibleColumn(column);
		// check, if this column was one of the sort columns
		if (column == fPrimarySortColumn)
			fPrimarySortColumn = NULL;
		if (column == fSecondarySortColumn)
			fSecondarySortColumn = NULL;
	}
	return column;
}

// _HideOrderedColumn
void
ColumnTreeView::_HideOrderedColumn(int32 index)
{
	_HideOrderedColumn(_OrderedColumnAt(index));
}

// _HideOrderedColumn
//
// If /column/ != NULL, it must be one of the list views columns.
void
ColumnTreeView::_HideOrderedColumn(Column* column)
{
	if (column && column->IsVisible()) {
		column->SetVisible(false);
		_RemoveVisibleColumn(column);
	}
}

// _ShowOrderedColumn
void
ColumnTreeView::_ShowOrderedColumn(int32 index)
{
	_ShowOrderedColumn(_OrderedColumnAt(index));
}

// _ShowOrderedColumn
//
// If /column/ != NULL, it must be one of the list views columns.
void
ColumnTreeView::_ShowOrderedColumn(Column* column)
{
	if (column && !column->IsVisible()) {
		column->SetVisible(true);
		_AddVisibleColumn(column);
	}
}

// _OrderedColumnAt
Column*
ColumnTreeView::_OrderedColumnAt(int32 index) const
{
	return (Column*)fColumns.ItemAt(index);
}

// OrderedColumnIndexOf
int32
ColumnTreeView::OrderedColumnIndexOf(Column* column) const
{
	return fColumns.IndexOf((void*)column);
}

// OrderedColumnIndexOf
int32
ColumnTreeView::OrderedColumnIndexOf(int32 index) const
{
	for (int32 i = 0; Column* column = _OrderedColumnAt(i); i++) {
		if (column->Index() == index)
			return i;
	}
	return -1;
}

// _SetPrimarySortOrderedColumn
void
ColumnTreeView::_SetPrimarySortOrderedColumn(int32 index, bool inverse)
{
	bool sort = false;
	if (Column* column = _OrderedColumnAt(index)) {
		bool columnInverse = (column->Flags() & COLUMN_SORT_INVERSE);
		if (column != fPrimarySortColumn) {
			// the column can't be both, primary and secondary sort column
			if (column == fSecondarySortColumn)
				_SetSecondarySortOrderedColumn(-1, false);
			Column* oldColumn = fPrimarySortColumn;
			fPrimarySortColumn = column;
			column->AddFlags(COLUMN_PRIMARY_SORT_KEY);
			if (inverse)
				column->AddFlags(COLUMN_SORT_INVERSE);
			if (oldColumn) {
				oldColumn->ClearFlags(COLUMN_PRIMARY_SORT_KEY
									  | COLUMN_SORT_INVERSE);
			}
			// ...
			// just to see an effect:
			fHeaderView->_InvalidateHeaders(0, -1);
			sort = true;
		} else if (inverse != columnInverse) {
			if (inverse)
				column->AddFlags(COLUMN_SORT_INVERSE);
			else
				column->ClearFlags(COLUMN_SORT_INVERSE);
			sort = true;
		}
	} else {
		// unset the sort column
		if (fPrimarySortColumn) {
			fPrimarySortColumn->ClearFlags(COLUMN_PRIMARY_SORT_KEY
										   | COLUMN_SORT_INVERSE);
		}
		fPrimarySortColumn = NULL;
	}
	_UpdateItemCompare(sort);
}

// _SetSecondarySortOrderedColumn
void
ColumnTreeView::_SetSecondarySortOrderedColumn(int32 index, bool inverse)
{
	bool sort = false;
	if (Column* column = _OrderedColumnAt(index)) {
		bool columnInverse = (column->Flags() & COLUMN_SORT_INVERSE);
		if (column == fPrimarySortColumn) {
			// the column can't be both, primary and secondary sort column
			// we do nothing
		} else if (column != fSecondarySortColumn) {
			Column* oldColumn = fSecondarySortColumn;
			fSecondarySortColumn = column;
			column->AddFlags(COLUMN_SECONDARY_SORT_KEY);
			if (inverse)
				column->AddFlags(COLUMN_SORT_INVERSE);
			if (oldColumn) {
				oldColumn->ClearFlags(COLUMN_SECONDARY_SORT_KEY
									  | COLUMN_SORT_INVERSE);
			}
			// ...
			// just to see an effect:
			fHeaderView->_InvalidateHeaders(0, -1);
			sort = true;
		} else if (inverse != columnInverse) {
			if (inverse)
				column->AddFlags(COLUMN_SORT_INVERSE);
			else
				column->ClearFlags(COLUMN_SORT_INVERSE);
			sort = true;
		}
	} else {
		// unset the sort column
		if (fSecondarySortColumn) {
			fSecondarySortColumn->ClearFlags(COLUMN_SECONDARY_SORT_KEY
											 | COLUMN_SORT_INVERSE);
		}
		fSecondarySortColumn = NULL;
	}
	_UpdateItemCompare(sort);
}

// _CalculateVisibleColumnIndex
//
// Calculates the position of /column/ in the list of visible columns
// based on the information from the column list. That is, all columns
// before /column/ must have a correct visibility flag, though /column/
// doesn't need to.
// No checking is done!
int32
ColumnTreeView::_CalculateVisibleColumnIndex(Column* column)
{
	int32 index = 0;
	for (int32 i = 0; Column* col = _OrderedColumnAt(i); i++) {
		if (col == column)
			break;
		if (col->IsVisible())
			index++;
	}
	return index;
}

// _AddVisibleColumn
//
// /column/ must already be in the columns list and will now be inserted
// at the correct position in the list of the visible columns.
// The header view is notified as well.
// No checking is done!
void
ColumnTreeView::_AddVisibleColumn(Column* column)
{
	// Get the insertion index.
	int32 index = _CalculateVisibleColumnIndex(column);
	// insert in visible list
	fVisibleColumns.AddItem((void*)column, index);
	// insert header in header view
	fHeaderView->AddHeader(column->Header(), index);
	// Update the graphics stuff.
	_UpdateColumnXOffsets(index, false);
	_InvalidateVisibleColumns(index, -1);
}

// _RemoveVisibleColumn
//
// Removes /column/ from the list of visible columns.
// The header view is notified as well.
// No checking is done!
void
ColumnTreeView::_RemoveVisibleColumn(Column* column)
{
	// Get the columns index and remove it.
	int32 index = _VisibleColumnIndexOf(column);
	fVisibleColumns.RemoveItem(index);
	// remove header
	fHeaderView->RemoveHeader(index);
	// Update the graphics stuff.
	_UpdateColumnXOffsets(index, true);
//	_InvalidateVisibleColumns(index, -1);
}

// _VisibleColumnAt
Column*
ColumnTreeView::_VisibleColumnAt(int32 index) const
{
	return (Column*)fVisibleColumns.ItemAt(index);
}

// _VisibleColumnIndexOf
int32
ColumnTreeView::_VisibleColumnIndexOf(Column* column) const
{
	return fVisibleColumns.IndexOf((void*)column);
}

// _VisibleColumnIndexOf
//
// Returns the index of the visible column that contains the supplied
// coordinates (measured in view coordinates).
// Returns -1 in case no such column exists.
int32
ColumnTreeView::_VisibleColumnIndexOf(BPoint point) const
{
	return _VisibleColumnIndexOf(point.x);
}

// _VisibleColumnIndexOf
//
// Returns the index of the visible column that contains the supplied
// X coordinate (measured in view coordinates).
// Returns -1 in case no such column exists.
int32
ColumnTreeView::_VisibleColumnIndexOf(float x) const
{
	// search column -- slightly inefficient
	for (int32 i = 0; Column* column = _VisibleColumnAt(i); i++) {
		BRect rect(_VisibleColumnFrame(column));
		if (rect.left <= x && rect.right >= x)
			return i;
	}
	return -1;
}

// _CountVisibleColumns
int32
ColumnTreeView::_CountVisibleColumns() const
{
	return fVisibleColumns.CountItems();
}

// _VisibleColumnPosition
//
// See _VisibleColumnPosition(Column*).
BPoint
ColumnTreeView::_VisibleColumnPosition(int32 index) const
{
	return _VisibleColumnPosition(_VisibleColumnAt(index));
}

// _VisibleColumnPosition
//
// Returns the suplied column's position (left top coordinates) measured in
// virtual scrolling coordinates, that is, it is correctly offset for
// directly being used for drawing.
// Returns an undefined position in case of a NULL pointer.
BPoint
ColumnTreeView::_VisibleColumnPosition(Column* column) const
{
	BPoint position;
	if (column) {
		position = _ItemsRect().LeftTop() - fCurrentScrollOffset;
		position.x += column->XOffset();
	}
	return position;
}

// _VisibleColumnFrame
//
// See _VisibleColumnFrame(Column*).
BRect
ColumnTreeView::_VisibleColumnFrame(int32 index) const
{
	return _VisibleColumnFrame(_VisibleColumnAt(index));
}

// _VisibleColumnFrame
//
// Returns the suplied column's frame measured in virtual scrolling
// coordinates, that is, its coordinates are correctly offset for directly
// being used for drawing.
// Returns an invalid rectangle in case of a NULL pointer.
BRect
ColumnTreeView::_VisibleColumnFrame(Column* column) const
{
	BRect frame;
	if (column) {
		frame.SetLeftTop(_VisibleColumnPosition(column));
		frame.right = frame.left + column->Width();
		frame.bottom = frame.top + DataRect().Height();
	}
	return frame;
}

// _ItemPosition
//
// See _ItemPosition(ColumnTreeItem*).
BPoint
ColumnTreeView::_ItemPosition(int32 index) const
{
	return _ItemPosition(ItemAt(index));
}

// _ItemPosition
//
// Returns the suplied item's position (left top coordinates) measured in
// virtual scrolling coordinates, that is, it is correctly offset for
// directly being used for drawing.
// Returns an undefined position in case of a NULL pointer.
BPoint
ColumnTreeView::_ItemPosition(ColumnTreeItem* item) const
{
	BPoint position;
	if (item) {
		position = _ItemsRect().LeftTop() - fCurrentScrollOffset;
		position.y += item->YOffset();
	}
	return position;
}

// _ItemFrame
//
// See _ItemFrame(ColumnTreeItem*).
BRect
ColumnTreeView::_ItemFrame(int32 index) const
{
	return _ItemFrame(ItemAt(index));
}

// _ItemFrame
//
// Returns the supplied item's frame measured in virtual scrolling coordinates,
// that is, its coordinates are correctly offset for directly being used for
// drawing.
// Returns an invalid rectangle in case of a NULL pointer.
BRect
ColumnTreeView::_ItemFrame(ColumnTreeItem* item) const
{
	BRect frame;
	if (item) {
		frame.SetLeftTop(_ItemPosition(item));
		frame.right = frame.left + DataRect().Width();
		frame.bottom = frame.top + item->Height();
	}
	return frame;
}

// _ItemIndentationFrame
//
// See _ItemIndentationFrame(ColumnTreeItem*).
BRect
ColumnTreeView::_ItemIndentationFrame(int32 index) const
{
	return _ItemIndentationFrame(ItemAt(index));
}

// _ItemIndentationFrame
//
// Returns the frame of the supplied item's indentation area (the area in the
// first column by which the item is indented, if the item's level is > 0)
// measured in virtual scrolling coordinates, that is, its coordinates are
// correctly offset for directly being used for drawing.
// Returns an invalid rectangle in case of a NULL pointer or if the item
// is not indented.
BRect
ColumnTreeView::_ItemIndentationFrame(ColumnTreeItem* item) const
{
	if (item && fModel && fItemHandle) {
		if (Column* column = _VisibleColumnAt(0)) {
			float indentation
				= fItemHandle->GetIndentation(fModel->LevelOf(item));
			BRect columnRect(_VisibleColumnFrame(column) & _ItemFrame(item));
			if (columnRect.Width() + 1 > indentation)
				columnRect.right = columnRect.left + indentation - 1;
			return columnRect;
		}
	}
	return BRect();
}

// _ItemHandleFrame
//
// See _ItemHandleFrame(ColumnTreeItem*).
BRect
ColumnTreeView::_ItemHandleFrame(int32 index) const
{
	return _ItemHandleFrame(ItemAt(index));
}

// _ItemHandleFrame
//
// Returns the supplied item's handle frame measured in virtual scrolling
// coordinates, that is, its coordinates are correctly offset for directly
// being used for drawing.
// Returns an invalid rectangle in case of a NULL pointer or if the item
// does not have children or for any other reason doesn't have a handle.
BRect
ColumnTreeView::_ItemHandleFrame(ColumnTreeItem* item) const
{
	if (item && fItemHandle)
		return fItemHandle->GetHandleRect(item, _ItemIndentationFrame(item));
	return BRect();
}

// _FindSelectedItem
//
// The supplied index must be valid and contained in the selection list.
int32
ColumnTreeView::_FindSelectedItem(int32 index) const
{
	return _FindSelectionInsertionIndex(index);
}

// _FindSelectionInsertionIndex
//
// The supplied index must be valid.
int32
ColumnTreeView::_FindSelectionInsertionIndex(int32 index) const
{
	// binary search
	int32 lower = 0;
	int32 upper = fSelectedItems.CountItems();
	while (lower < upper) {
		int32 mid = (lower + upper) / 2;
		int32 midIndex = CurrentSelection(mid);
		if (index <= midIndex)
			upper = mid;
		else
			lower = mid + 1;
	}
	return lower;
}

// _InternalSelectionChanged
void
ColumnTreeView::_InternalSelectionChanged()
{
	if (fSelectionDepth == 0) {
		_InternalInvoke(fSelectionMessage);
		SelectionChanged();
	}
}

// _InternalInvoke
void
ColumnTreeView::_InternalInvoke(const BMessage* message)
{
	if (message) {
		BMessage clone(*message);
		clone.AddInt64("when", system_time());
		clone.AddPointer("source", (void*)this);
		int32 count = fSelectedItems.CountItems();
		for (int32 i = 0; i < count; i++)
			clone.AddInt32("index", CurrentSelection(i));
		InvokeNotify(&clone);
	}
}

// _ReindexColumns
//
// Adds /offset/ to each column index >= /index/.
void
ColumnTreeView::_ReindexColumns(int32 index, int32 offset)
{
	for (int32 i = 0; Column* column = _OrderedColumnAt(i); i++) {
		if (column->Index() >= index)
			column->SetIndex(column->Index() + offset);
	}
}

// _ReindexSelectedItems
//
// Adds /offset/ to each selected item index >= /index/.
void
ColumnTreeView::_ReindexSelectedItems(int32 index, int32 offset)
{
	int32 count = fSelectedItems.CountItems();
	for (int32 i = 0; i < count; i++) {
		int32 item = CurrentSelection(i);
		if (item >= index) {
			fSelectedItems.Remove(i);
			fSelectedItems.Add(item + offset + 1, i);
		}
	}
}

// _RebuildSelectionList
//
// Empties the selection list and rebuilds it from the items list and the
// items selected flag.
void
ColumnTreeView::_RebuildSelectionList()
{
	fSelectedItems.Clear();
	for (int32 i = 0; ColumnTreeItem* item = ItemAt(i); i++) {
		if (item->IsSelected())
			fSelectedItems.Add(i + 1);
	}
}

// _UpdateColumnXOffsets
void
ColumnTreeView::_UpdateColumnXOffsets(int32 index, bool update)
{
	int32 count = CountColumns();
	if (index < 0)
		index = 0;
	if (index > count)
		index = count;
	int32 firstDifference = -1;
	int32 lastDifference = -1;
	Column* column = _VisibleColumnAt(index - 1);
	float offset = (column) ? column->XOffset() + column->Width() + 1.0 : 0.0;
	for (; Column* column = _VisibleColumnAt(index); index++) {
		if (offset != column->XOffset()) {
			if (firstDifference == - 1)
				firstDifference = index;
			lastDifference = index;
		}
		column->SetXOffset(offset);
		offset += column->Width() + 1.0f;
	}
	// update list view width
	BRect dataRect(DataRect());
	dataRect.right = dataRect.left + offset - 1.0f;
	SetDataRect(dataRect);
//	if (list width != offset) {
//		if (firstDifference == - 1)
//			firstDifference = index;
//		lastDifference = index;
//	}
	// force an update to validate the invalid columns
	if (update && firstDifference != -1) {
		_InvalidateVisibleColumns(firstDifference,
								  lastDifference - firstDifference + 1);
	}
}

// _UpdateItemYOffsets
void
ColumnTreeView::_UpdateItemYOffsets(int32 index, bool update)
{
	int32 count = CountItems();
	if (index < 0)
		index = 0;
	if (index > count)
		index = count;
	int32 firstDifference = -1;
	int32 lastDifference = -1;
	ColumnTreeItem* item = ItemAt(index - 1);
	float offset = (item) ? item->YOffset() + item->Height() + 1.0f : 0.0f;
	for (; ColumnTreeItem* item = ItemAt(index); index++) {
		if (offset != item->YOffset()) {
			if (firstDifference == - 1)
				firstDifference = index;
			lastDifference = index;
		}
		item->SetYOffset(offset);
		offset += item->Height() + 1.0f;
	}
	// update list view height
	BRect dataRect(DataRect());
	dataRect.bottom = dataRect.top + offset - 1.0f;
	SetDataRect(dataRect);
//	if (list height != offset) {
//		if (firstDifference == - 1)
//			firstDifference = index;
//		lastDifference = index;
//	}
	// force an update to validate the invalid columns
	if (update && firstDifference != -1) {
		_InvalidateItems(firstDifference,
						 lastDifference - firstDifference + 1);
	}
}

// _UpdateVisibleColumnTree
/*void
ColumnTreeView::_UpdateVisibleColumnTree(bool update)
{
	// Keep a copy of the old list for comparison.
	BList oldBVisibleList(fVisibleList);
	fVisibleList.MakeEmpty();
	// build the new list and find out where it differs first from the old one
	int32 index = 0;
	int32 firstDifference = -1;
	for (int32 i = 0; Column* column = _OrderedColumnAt(i); i++) {
		if (column->IsVisible()) {
			fVisibleList.AddItem((void*)column);
			if (firstDifference == -1 &&
				oldVisibleList.ItemAt(index) == (void*)column) {
				fistDifference = index;
			}
			index++;
		}
	}
	// the old list is longer than the new one
//	if (firstDifference == -1 &&
//		fVisibleList.CountItems() < oldVisibleList.CountItems()) {
//		firstDifference = fVisibleList.CountItems();
//	}
	if (firstDifference != -1)
		_UpdateColumnXOffsets(firstDifference, false);
	// force an update to validate the invalid columns
	if (update && firstDifference != -1) {
		_InvalidateItems(firstDifference,
						 lastDifference - firstDifference + 1);
	}
}*/

// _InvalidateVisibleColumns
void
ColumnTreeView::_InvalidateVisibleColumns(int32 index, int32 count)
{
	// nothing to do
	if (count == 0)
		return;
	int32 columnCount = _CountVisibleColumns();
	BRect itemsRect(_ItemsRect());
	BRect rect(itemsRect);
	if (index < 0 || index >= columnCount) {
		// invalidate the region on right of the last column
		if (Column* column = _VisibleColumnAt(columnCount - 1))
			rect.left = _VisibleColumnFrame(column).right + 1.0f;
	} else {
		if (Column* column = _VisibleColumnAt(index))
			rect.left = _VisibleColumnPosition(column).x;
		if (count > 0) {
			if (index + count > columnCount)
				count = columnCount - index;
			if (Column* column = _VisibleColumnAt(index + count - 1))
				rect.right = _VisibleColumnFrame(column).right;
		}
	}
	rect = rect & itemsRect;
	if (rect.IsValid())
		Invalidate(rect);
}

// _InvalidateItems
//
// Invalidates /count/ items starting at /index/. If /index/ < 0 or
// >= the item count the region below the last item is invalidated.
// Otherwise if count < 0 the whole region starting at index is invalidated.
void
ColumnTreeView::_InvalidateItems(int32 index, int32 count)
{
	// nothing to do
	if (count == 0)
		return;
	int32 itemCount = CountItems();
	BRect itemsRect(_ItemsRect());
	BRect rect(itemsRect);
	if (index < 0 || index >= itemCount) {
		// invalidate the region below the last item
		if (ColumnTreeItem* item = ItemAt(itemCount - 1))
			rect.top = _ItemFrame(item).bottom + 1.0f;
	} else {
		if (ColumnTreeItem* item = ItemAt(index))
			rect.top = _ItemPosition(item).y;
		if (count > 0) {
			if (index + count > itemCount)
				count = itemCount - index;
			if (ColumnTreeItem* item = ItemAt(index + count - 1))
				rect.bottom = _ItemFrame(item).bottom;
		}
	}
	rect = rect & itemsRect;
	if (rect.IsValid())
		Invalidate(rect);
}

// _Layout
void
ColumnTreeView::_Layout()
{
	BRect rect(_HeaderViewRect());
	fHeaderView->MoveTo(rect.LeftTop());
	fHeaderView->ResizeTo(rect.Width(), rect.Height());
}

// _DrawItem
void
ColumnTreeView::_DrawItem(ColumnTreeItem* item, BView* view, int32 firstColumn,
						  int32 lastColumn, BRect updateRect)
{
//printf("ColumnTreeView::_DrawItem(..., %ld, %ld,...)\n", firstColumn,
//lastColumn);
	if (!item || !fModel)
		return;
	BRect itemRect(_ItemFrame(item));
	int32 level = fModel->LevelOf(item);
//printf("  item level: %ld\n", level);
	// draw the first column separately, if the item level is greater than 0
	if (firstColumn == 0 && level > 0) {
//printf("  draw first column\n");
		Column* column = _VisibleColumnAt(firstColumn);
		firstColumn++;
		BRect columnRect(_VisibleColumnFrame(column) & itemRect);
		float indentation = (fItemHandle ? fItemHandle->GetIndentation(level)
										 : 20);
		// draw the item
		BRect indentedColumnRect(columnRect.LeftTop() + BPoint(indentation, 0),
								 columnRect.RightBottom());
//printf("  indentedColumnRect");
//indentedColumnRect.PrintToStream();
		item->Draw(view, column, indentedColumnRect,
				   indentedColumnRect & updateRect,
				   item->Flags(), &fColors->item_colors);
//		// draw the background of the indentation area
		BRect indentationArea(columnRect.left, columnRect.top,
							  indentedColumnRect.left - 1, columnRect.bottom);
		if (indentationArea.IsValid()) {
			uint32 flags = fState->ItemFlags(item);
			if (fItemHandle) {
				fItemHandle->Draw(view, item, column, indentationArea,
					indentationArea & updateRect, flags,
					&fColors->item_colors);
			} else {
				item->DrawBackground(view, column, indentedColumnRect,
					indentationArea, flags, &fColors->item_colors);
			}
		}
/*
printf("  indentationArea");
indentationArea.PrintToStream();
		item->DrawBackground(view, column, indentedColumnRect,
							 indentationArea, item->Flags(),
							 &fColors->item_colors);
		// draw the handle, if the item has children
printf("  item children: %ld\n", fModel->CountSubItems(item));
		if (fModel->CountSubItems(item)) {
printf("  draw handle\n");
			float right = indentedColumnRect.left - 1;
			float left = indentedColumnRect.left - kIndentationPerLevel;
			float top = indentedColumnRect.top;
			float bottom = indentedColumnRect.bottom;
			float midH = (left + right) / 2;
			float midV = (top + bottom) / 2;
			SetHighColor(fColors->item_colors.foreground);
			if (item->IsExpanded()) {
printf("handle: (%f, %f), (%f, %f), (%f, %f)\n", left, top, right, top, midH,
bottom);
				view->FillTriangle(BPoint(left, top), BPoint(right, top),
								   BPoint(midH, bottom));
			} else {
				view->FillTriangle(BPoint(left, top), BPoint(right, midV),
								   BPoint(left, bottom));
			}
		}
*/
	}
	// draw the other columns
	for (int32 c = firstColumn; c <= lastColumn; c++) {
		Column* column = _VisibleColumnAt(c);
		BRect columnRect(_VisibleColumnFrame(column) & itemRect);
		item->Draw(view, column, columnRect, columnRect & updateRect,
				   item->Flags(), &fColors->item_colors);
	}
}


// _HeaderViewRect
//
// Returns the rectangle that contains the headers, measured in virtual
// scrolling coordinates, that is its coordinates are correctly offset for
// directly being used for drawing the headers.
BRect
ColumnTreeView::_HeaderViewRect() const
{
	BRect rect;
	rect.left = -fCurrentScrollOffset.x;
	rect.right = rect.left + MAX(DataRect().Width(), Bounds().Width());
	rect.top = 0.0f;
	rect.bottom = rect.top + fHeaderView->Height();
	return rect;
}

// _ItemsRect
//
// Returns the part of the view, that is intended to display the items
// (measured in view coorinates).
BRect
ColumnTreeView::_ItemsRect() const
{
	BRect rect(Bounds());
	rect.top += fHeaderView->Height() + 1.0f;
	return rect;
}

// _ActualItemsRect
//
// Returns the part of the _ItemsRect() that is actually covered by items.
BRect
ColumnTreeView::_ActualItemsRect() const
{
	BRect rect(_ItemsRect());
	if (_CountVisibleColumns() == 0 || CountItems() == 0) {
		rect.right = rect.left - 1.0f;
		rect.bottom = rect.top - 1.0f;
	} else {
		Column* lastColumn = _VisibleColumnAt(_CountVisibleColumns() - 1);
		ColumnTreeItem* lastItem = ItemAt(CountItems() - 1);
		rect.right = MIN(rect.right, _VisibleColumnFrame(lastColumn).right);
		rect.bottom = MIN(rect.bottom, _ItemFrame(lastItem).bottom);
	}
	return rect;
}

// _UpdateItemCompare
void
ColumnTreeView::_UpdateItemCompare(bool resort)
{
	if (resort && fModel)
		fModel->SetSortCompareFunction(NULL);
	fItemCompare->SetTo(fCompareFunction, fPrimarySortColumn,
						fSecondarySortColumn);
	if (fModel && fModel->GetSortCompareFunction() != fItemCompare)
		fModel->SetSortCompareFunction(fItemCompare);
}

// _ChangeState
void
ColumnTreeView::_ChangeState(State* state)
{
	delete fState;
	fState = state;
}

