// ColumnTreeViewColors.h

#ifndef COLUMN_TREE_VIEW_COLORS_H
#define COLUMN_TREE_VIEW_COLORS_H

#include <GraphicsDefs.h>

// item colors
struct column_tree_item_colors {
	rgb_color					foreground;
	rgb_color					background;
	rgb_color					highlight;
	rgb_color					shadow;
	rgb_color					selected_foreground;
	rgb_color					selected_background;
	rgb_color					selected_highlight;
	rgb_color					selected_shadow;
};

// header colors
struct column_header_colors {
	rgb_color					foreground;
	rgb_color					background;
	rgb_color					highlight;
	rgb_color					light_shadow;
	rgb_color					shadow;
	rgb_color					pressed_foreground;
	rgb_color					pressed_background;
	rgb_color					pressed_highlight;
	rgb_color					pressed_shadow;
};

// header view colors
struct column_header_view_colors {
	rgb_color					background;
	column_header_colors		header_colors;
};

// tree view colors
struct column_tree_view_colors{
	rgb_color					background;
	column_tree_item_colors		item_colors;
	column_header_view_colors	header_view_colors;
};

// defaults
extern const column_tree_item_colors	kDefaultColumnTreeItemColors;
extern const column_header_colors		kDefaultColumnHeaderColors;
extern const column_header_view_colors	kDefaultColumnHeaderViewColors;
extern const column_tree_view_colors	kDefaultColumnTreeViewColors;


#endif	// COLUMN_TREE_VIEW_COLORS_H
