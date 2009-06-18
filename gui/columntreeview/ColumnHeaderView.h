// ColumnHeaderView.h

#ifndef COLUMN_HEADER_VIEW_H
#define COLUMN_HEADER_VIEW_H

#include <List.h>
#include <View.h>

#include "ColumnHeaderViewStates.h"

class BCursor;

class ColumnHeader;
class ColumnTreeView;
struct column_header_view_colors;

class ColumnHeaderView : public BView {
 public:
								ColumnHeaderView();
	virtual						~ColumnHeaderView();

	virtual	void				Draw(BRect updateRect);
	virtual	void				MouseDown(BPoint point);
	virtual	void				MouseMoved(BPoint point, uint32 transit,
										   const BMessage* message);
	virtual	void				MouseUp(BPoint point);


			void				SetParentView(ColumnTreeView* parent);
			ColumnTreeView*		ParentView() const;

			float				Height() const;

			void				AddHeader(ColumnHeader* header, int32 index);
			void				MoveHeader(int32 oldIndex, int32 newIndex);
			void				MoveHeaders(int32 index, int32 dest,
											int32 count);
			ColumnHeader*		RemoveHeader(int32 index);
			bool				RemoveHeader(ColumnHeader* header);
			void				ResizeHeader(int32 index, float width);
			ColumnHeader*		HeaderAt(int32 index) const;
			ColumnHeader*		HeaderAt(BPoint point) const;
			int32				IndexOf(ColumnHeader* header) const;
			int32				IndexOf(BPoint point) const;
			int32				IndexOf(float x) const;
			int32				CountHeaders() const;

			// colors
			void				SetColors(
									const column_header_view_colors* colors);
			const column_header_view_colors* Colors() const;

 private:
			ColumnTreeView*		fParentView;
			BList				fHeaders;
			BList				fVisibleHeaders;
			float				fHeight;
			BCursor*			fHorizontalResizeCursor;
			ColumnHeaderViewStates::State*	fState;
			column_header_view_colors*	fColors;

			ColumnHeader*		_VisibleHeaderAt(int32 i) const;
			int32				_CountVisibleHeaders() const;
			int32				_VisibleHeaderIndexFor(int32 index) const;
			ColumnHeader*		_VisibleHeaderFor(int32 index) const;
			int32				_VisibleIndexOf(ColumnHeader* header) const;

			void				_InvalidateHeaders(int32 index,
												   int32 count = -1);

			void				_InvalidateVisibleHeaders(int32 index,
												   		  int32 count = -1);

			BRect				_HeaderRect(ColumnHeader* header) const;
			BRect				_HeaderRect(int32 index) const;
			BRect				_HeadersRect() const;

			void				_RebuildVisibleHeaders();

			void				_ChangeState(ColumnHeaderViewStates::State*
											 state);

			void				_SetHorizontalResizeCursor();
			void				_SetStandardCursor();

	friend class ColumnTreeView;
	friend class ColumnHeaderViewStates::State;
	friend class ColumnHeaderViewStates::DraggingState;
	friend class ColumnHeaderViewStates::IgnoreState;
	friend class ColumnHeaderViewStates::InsideState;
	friend class ColumnHeaderViewStates::OutsideState;
	friend class ColumnHeaderViewStates::PressedState;
	friend class ColumnHeaderViewStates::ResizingState;
};



#endif	// COLUMN_HEADER_VIEW_H
