// ColumnTreeView.h

#ifndef COLUMN_TREE_VIEW_H
#define COLUMN_TREE_VIEW_H

#include <Invoker.h>
#include <List.h>
#include <View.h>

#include "ColumnTreeModelListener.h"
#include "ColumnTreeViewStates.h"
#include "Scrollable.h"

class Column;
class ColumnHeaderView;
class ColumnTreeItem;
class ColumnTreeItemHandle;
class ColumnTreeModel;
struct column_tree_view_colors;

// item compare function type
typedef int column_tree_compare_function(const ColumnTreeItem*,
										 const ColumnTreeItem*,
										 const Column*);

enum selection_mode {
	CLV_SINGLE_SELECTION,
	CLV_MULTIPLE_SELECTION,
};

class ColumnTreeView : public BView, public BInvoker, public Scrollable,
					   private ColumnTreeModelListener {
public:
								ColumnTreeView(BRect frame);
#ifdef __HAIKU__
								ColumnTreeView();
#endif
	virtual						~ColumnTreeView();

	virtual	void				AttachedToWindow();
	virtual	void				Draw(BRect updateRect);
	virtual	void				Draw(BView* view, BRect updateRect);
	virtual	void				FrameResized(float width, float height);
	virtual	void				KeyDown(const char* bytes, int32 numBytes);
	virtual	void				MakeFocus(bool focusState = true);
	virtual	void				MouseDown(BPoint point);
	virtual	void				MouseMoved(BPoint point, uint32 transit,
										   const BMessage* message);
	virtual	void				MouseUp(BPoint point);

	virtual	void				GetPreferredSize(float* _width,
									float* _height);

#ifdef __HAIKU__
	virtual	BSize				MinSize();
	virtual	BSize				MaxSize();
#endif

//	virtual	status_t			Invoke(BMessage* message = NULL);

	virtual	void				ScrollOffsetChanged(BPoint oldOffset,
													BPoint newOffset);

			void				SetModel(ColumnTreeModel* model);
			ColumnTreeModel*	GetModel() const;

	// column management
			void				AddColumn(Column* column, int32 index = -1);
			Column*				RemoveColumn(int32 index);
			bool				RemoveColumn(Column* column);
//			bool				RemoveColumns(int32 index, int32 count);
			Column*				FindColumn(const char* name) const;
			Column*				ColumnAt(int32 index) const;
			int32				ColumnIndexOf(Column* column) const;
//			int32				ColumnIndexOf(BPoint point) const;
			int32				OrderedColumnIndexOf(Column* column) const;
			int32				OrderedColumnIndexOf(int32 index) const;
			int32				CountColumns() const;
			void				HideColumn(int32 index);
			void				HideColumn(Column* column);
			void				ShowColumn(int32 index);
			void				ShowColumn(Column* column);

			void				InvalidateHeader(int32 index);

	// item management (list of visible items)
	virtual	void				AddItem(ColumnTreeItem* item,
										int32 index = -1);
		// TODO: return value should be bool
	virtual	ColumnTreeItem*		RemoveItem(int32 index);
	virtual	bool				RemoveItem(ColumnTreeItem* item);
	virtual	bool				RemoveItems(int32 index, int32 count);
			bool				HasItem(ColumnTreeItem* item) const;
			ColumnTreeItem*		ItemAt(int32 index) const;
			int32				IndexOf(ColumnTreeItem* item) const;
			int32				IndexOf(BPoint) const;
			int32				IndexOf(float y) const;
			int32				CountItems() const;
			bool				IsEmpty() const;
			BRect				ItemFrame(int32 index) const
										{ return _ItemFrame(index); }
			float				IndentationOf(ColumnTreeItem* item) const;
			float				IndentationOf(int32 level) const;
	virtual	void				MakeEmpty();
		// TODO: return value should be bool

	// item management (tree of all items)
// TODO: Make these methods virtual? For we now use a separate tree model
// and a listener mechanism, that's not really needed anymore.
			bool				AddSubItem(ColumnTreeItem* super,
										   ColumnTreeItem* item);
			bool				AddSubItem(ColumnTreeItem* super,
										   ColumnTreeItem* item,
										   int32 index);
			ColumnTreeItem*		RemoveSubItem(ColumnTreeItem* super,
											  int32 index);
			bool				RemoveSubItems(ColumnTreeItem* super,
											   int32 index, int32 count);
			int32				CountSubItems(ColumnTreeItem* super);
			int32				CountSubItemsRecursive(ColumnTreeItem* super,
									bool visibleItemsOnly);
			ColumnTreeItem*		SubItemAt(ColumnTreeItem* super,
										  int32 index);
			int32				SubItemIndexOf(ColumnTreeItem* item);
			ColumnTreeItem*		SuperItemOf(ColumnTreeItem* item);
			int32				LevelOf(ColumnTreeItem* item);

			void				InvalidateItem(int32 index);
			void				InvalidateItem(ColumnTreeItem* item);
			void				ItemChanged(int32 index);
			void				ItemChanged(ColumnTreeItem* item);
			void				ScrollToItem(int32 index);
	virtual	bool				InitiateDrag(BPoint point, int32 index,
									bool wasSelected,
									BMessage* _message = NULL);
	virtual	bool				GetDropInfo(BPoint point,
									const BMessage& dragMessage,
									ColumnTreeItem** super,
									int32* _subItemIndex, int32* _itemIndex);
	virtual	void				HandleDrop(const BMessage& dragMessage,
									ColumnTreeItem* super, int32 subItemIndex,
									int32 itemIndex);
	virtual	void				ItemDoubleClicked(int32 index);

	// item visibility
			bool				ExpandItem(ColumnTreeItem* item);
			bool				ExpandItem(int32 index);
			bool				CollapseItem(ColumnTreeItem* item);
			bool				CollapseItem(int32 index);

	// selection
			void				SetSelectionMode(selection_mode mode);
	inline	selection_mode		SelectionMode() const
										{ return fSelectionMode; }
			int32				CurrentSelection(int32 index = 0) const;
			void				Deselect(int32 index);
			void				DeselectAll();
			void				DeselectExcept(int32 start, int32 finish);
			bool				IsItemSelected(int32 index) const;
			void				ScrollToSelection();
			void				Select(int32 index, bool extend = false);
			void				Select(int32 start, int32 finish,
									   bool extend = false);
	virtual	void				SelectionChanged();
			int32				CountSelectedItems() const;

	virtual void				SetInvocationMessage(BMessage* message);
			BMessage*			InvocationMessage() const;
			uint32				InvocationCommand() const;
	virtual	void				SetSelectionMessage(BMessage* message);
			BMessage*			SelectionMessage() const;
			uint32				SelectionCommand() const;

			void				ResizeColumn(int32 index, float width);
			void				ResizeColumn(Column* column, float width);
			void				MoveColumn(int32 index, int32 dest);
			void				MoveColumn(Column* column, int32 dest);

	// sorting
			status_t			PrimarySortColumn(int32 *outIndex,
												  bool *outInverse) const;
			status_t			SecondarySortColumn(int32 *outIndex,
													bool *outInverse) const;

			void				SetPrimarySortColumn(int32 index,
													 bool inverse = false);
			void				SetSecondarySortColumn(int32 index,
													   bool inverse = false);
			void				SetSortCompareFunction(
									column_tree_compare_function* function);
			void				Sort();

	// colors
			void				SetColors(
									const column_tree_view_colors* colors);
			const column_tree_view_colors*	Colors() const;

	// private in principle, but called by the header view
			void				ResizeVisibleColumn(int32 index, float width);
			void				MoveVisibleColumn(int32 oldIndex,
												  int32 newIndex);
			void				MoveVisibleColumns(int32 index, int32 dest,
												   int32 count);

			void				DisableScrolling();
			void				EnableScrolling();

			void				SetPrimarySortVisibleColumn(int32 index,
															bool inverse);
			void				SetSecondarySortVisibleColumn(int32 index,
															  bool inverse);
 private:
 	// ColumnTreeModelListener implementation
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

 private:
	friend class ColumnTreeViewStates::State;
	friend class ColumnTreeViewStates::DraggingState;
	friend class ColumnTreeViewStates::IgnoreState;
	friend class ColumnTreeViewStates::InsideState;
	friend class ColumnTreeViewStates::OutsideState;
	friend class ColumnTreeViewStates::PressedState;
//	friend class ColumnTreeViewStates::ResizingState;

	class ItemCompare;

private:
			void				_Init();

			Column*				_RemoveOrderedColumn(int32 index);
			void				_HideOrderedColumn(int32 index);
			void				_HideOrderedColumn(Column* column);
			void				_ShowOrderedColumn(int32 index);
			void				_ShowOrderedColumn(Column* column);
			Column*				_OrderedColumnAt(int32 index) const;

			void				_SetPrimarySortOrderedColumn(int32 index,
															 bool inverse);
			void				_SetSecondarySortOrderedColumn(int32 index,
															   bool inverse);

			int32				_CalculateVisibleColumnIndex(Column* column);
			void				_AddVisibleColumn(Column* column);
			void				_RemoveVisibleColumn(Column* column);

protected:
			Column*				_VisibleColumnAt(int32 index) const;
			int32				_VisibleColumnIndexOf(Column* column) const;
			int32				_VisibleColumnIndexOf(BPoint point) const;
			int32				_VisibleColumnIndexOf(float x) const;
			int32				_CountVisibleColumns() const;

			BPoint				_VisibleColumnPosition(int32 index) const;
			BPoint				_VisibleColumnPosition(Column* column) const;
			BRect				_VisibleColumnFrame(int32 index) const;
			BRect				_VisibleColumnFrame(Column* column) const;

private:
			BPoint				_ItemPosition(int32 index) const;
			BPoint				_ItemPosition(ColumnTreeItem* item) const;
			BRect				_ItemFrame(int32 index) const;
			BRect				_ItemFrame(ColumnTreeItem* item) const;
			BRect				_ItemIndentationFrame(int32 index) const;
			BRect				_ItemIndentationFrame(
									ColumnTreeItem* item) const;
			BRect				_ItemHandleFrame(int32 index) const;
			BRect				_ItemHandleFrame(ColumnTreeItem* item) const;

			int32				_FindSelectedItem(int32 index) const;
			int32				_FindSelectionInsertionIndex(
										int32 index) const;

			void				_InternalSelectionChanged();
			void				_InternalInvoke(const BMessage* message);

			void				_ReindexColumns(int32 index, int32 offset);
			void				_ReindexSelectedItems(int32 index,
													  int32 offset);
			void				_RebuildSelectionList();

			void				_UpdateColumnXOffsets(int32 index,
													  bool update);
			void				_UpdateItemYOffsets(int32 index, bool update);
//			void				_UpdateVisibleColumnTree(bool update);

			void				_InvalidateVisibleColumns(int32 index,
														  int32 count);
			void				_InvalidateItems(int32 index, int32 count);

			void				_Layout();

			void				_DrawItem(ColumnTreeItem* item, BView* view,
										  int32 firstColumn, int32 lastColumn,
										  BRect updateRect);

			BRect				_HeaderViewRect() const;
			BRect				_ItemsRect() const;
			BRect				_ActualItemsRect() const;

	// sorting
			void				_UpdateItemCompare(bool resort);

	// state stuff
			void				_ChangeState(ColumnTreeViewStates::State*
											 state);

private:
			BList				fColumns;
			BList				fVisibleColumns;
			ColumnTreeModel*	fModel;
			BList				fSelectedItems;
			ColumnHeaderView*	fHeaderView;
			ColumnTreeItemHandle*	fItemHandle;
			BPoint				fCurrentScrollOffset;
			bool				fScrollingEnabled;
			Column*				fPrimarySortColumn;
			Column*				fSecondarySortColumn;
			BMessage*			fInvocationMessage;
			BMessage*			fSelectionMessage;
			BMessage*			fDragMessage;
			int32				fSelectionDepth;	// to avoid multiple msgs
			ColumnTreeViewStates::State*	fState;
			column_tree_view_colors*	fColors;
			column_tree_compare_function*	fCompareFunction;
			ItemCompare*		fItemCompare;
			selection_mode		fSelectionMode;
			int32				fLastClickedIndex;
			bigtime_t			fLastClickTime;
			bigtime_t			fDoubleClickTime;
};

#endif	// COLUMN_TREE_VIEW_H

