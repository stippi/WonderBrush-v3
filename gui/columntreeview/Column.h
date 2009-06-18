// Column.h

#ifndef COLUMN_H
#define COLUMN_H

#include <String.h>

// flags
enum {
	// flags accepted by the constructor
	COLUMN_MOVABLE				= 0x000001,
	COLUMN_VISIBLE				= 0x000002,
	COLUMN_JOIN_LEFT			= 0x000004,
	COLUMN_JOIN_RIGHT			= 0x000008,
	COLUMN_SORT_KEYABLE			= 0x000010,
	COLUMN_USER_FLAGS			= 0x0000ff,
	// internal flags
	COLUMN_PRESSED				= 0x000100,
	COLUMN_RESIZING				= 0x000200,
	COLUMN_MOVING				= 0x000400,
	COLUMN_PRIMARY_SORT_KEY		= 0x000800,
	COLUMN_SECONDARY_SORT_KEY	= 0x001000,
	COLUMN_SORT_INVERSE			= 0x002000,
};

// misc constants
enum {
	COLUMN_MIN_WIDTH	= 0,
	COLUMN_MAX_WIDTH	= 100000,
};

class ColumnHeader;
class ColumnTreeView;

class Column {
 public:
								Column(const char* label, const char* name,
									   float width, uint32 flags);
								Column(ColumnHeader* header, const char* name,
									   float width, uint32 flags);
	virtual						~Column();

			const char*			Name() const;

			void				SetHeader(ColumnHeader* header);
			ColumnHeader*		Header() const;

			void				InvalidateHeader();

			void				SetIndex(int32 index);
			int32				Index() const;

			void				SetXOffset(float offset);
			float				XOffset() const;

			void				SetWidth(float width);
			float				Width() const;

			void				SetWidthLimits(float minWidth, float maxWidth);
			float				MinWidth() const;
			float				MaxWidth() const;

			void				SetFlags(uint32 flags);
			uint32				Flags() const;
			void				AddFlags(uint32 flags);
			void				ClearFlags(uint32 flags);
			bool				IsMovable() const;
			bool				IsSortKeyable() const;

			void				SetVisible(bool visible);
			bool				IsVisible() const;

			// SetParent() is called by the list view only.
			void				SetParent(ColumnTreeView* view);
			ColumnTreeView*		Parent() const;

 private:
 			ColumnTreeView*		fParent;
			ColumnHeader*		fHeader;
			BString				fName;
			int32				fIndex;
			float				fXOffset;
			float				fWidth;
			float				fMinWidth;
			float				fMaxWidth;
			uint32				fFlags;
};



#endif	// COLUMN_H
