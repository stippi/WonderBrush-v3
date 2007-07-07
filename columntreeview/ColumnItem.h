// ColumnItem.h

#ifndef COLUMN_ITEM_H
#define COLUMN_ITEM_H

#include <Rect.h>
#include <String.h>

class BBitmap;
class BView;

class Column;
struct column_tree_item_colors;

// ColumnItem
class ColumnItem {
 public:
								ColumnItem(bool clearBackground = true);
	virtual						~ColumnItem();

	virtual	void				Draw(BView* view, Column* column, BRect frame,
									 BRect updateRect, uint32 flags,
									 const column_tree_item_colors* colors);

	inline	bool				ClearBackground() const
									{ return fClearBackground; }

 private:
			bool				fClearBackground;
};

// BitmapColumnItem
class BitmapColumnItem : public ColumnItem {
 public:
								BitmapColumnItem(const BBitmap* bitmap);
	virtual						~BitmapColumnItem();

	virtual	void				Draw(BView* view, Column* column, BRect frame,
									 BRect updateRect, uint32 flags,
									 const column_tree_item_colors* colors);

 private:
			BBitmap*			fBitmap;
};

// TextColumnItem
class TextColumnItem : public ColumnItem {
 public:
								TextColumnItem(const char* text);
	virtual						~TextColumnItem();

	virtual	void				Draw(BView* view, Column* column, BRect frame,
									 BRect updateRect, uint32 flags,
									 const column_tree_item_colors* colors);

			const char*			Text() const { return fText.String(); }

 private:
			BString				fText;
};

// DisableTextColumnItem
class DisabledTextColumnItem : public TextColumnItem {
 public:
								DisabledTextColumnItem(const char* text);
	virtual						~DisabledTextColumnItem();

	virtual	void				Draw(BView* view, Column* column, BRect frame,
									 BRect updateRect, uint32 flags,
									 const column_tree_item_colors* colors);
};

#endif	// COLUMN_ITEM_H
