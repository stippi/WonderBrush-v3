// ColumnHeader.h

#ifndef COLUMN_HEADER_H
#define COLUMN_HEADER_H

#include <Rect.h>

#include "Column.h"

class BView;

//class Column;
struct column_header_colors;

class ColumnHeader {
 public:
								ColumnHeader();
	virtual						~ColumnHeader();

	virtual	void				Draw(BView* view, BRect frame,
									 BRect updateRect, uint32 flags,
									 const column_header_colors* colors);

			void				DrawBackground(BView* view, BRect frame,
										BRect rect, uint32 flags,
										const column_header_colors* colors);
			void				DrawFrame(BView* view, BRect frame,
										  uint32 flags,
										  const column_header_colors* colors);

			void				Invalidate();

			// SetColumn() is called by the column only.
			void				SetParentColumn(Column* column);
			Column*				ParentColumn() const;

	inline	float				ColumnXOffset() const
									{ return fParentColumn->XOffset(); }
	inline	float				ColumnWidth() const
									{ return fParentColumn->Width(); }
	inline	uint32				Flags() const
									{ return fParentColumn->Flags(); }

			void				SetXOffset(float offset);
			float				XOffset() const;
			void				SetWidth(float width);
			float				Width() const;

			void				SetHeaderRange(int32 first, int32 last);
			void				SetFirstHeader(int32 first);
			void				SetLastHeader(int32 last);
			int32				FirstHeader() const;
			int32				LastHeader() const;
			int32				RangeSize() const;
			void				SetVisibleHeader(int32 index);
			int32				VisibleHeader() const;

 private:
			Column*				fParentColumn;
			float				fXOffset;
			float				fWidth;
			int32				fFirstHeader;
			int32				fLastHeader;
			int32				fVisibleHeader;
};


#endif	// COLUMN_HEADER_H
