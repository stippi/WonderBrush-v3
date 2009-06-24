// ColumnTreeViewColors.cpp

#include <InterfaceDefs.h>

#include "ColumnTreeViewColors.h"

#define BLUE_PERCENTAGE 0.03

const rgb_color backGround = ui_color(B_PANEL_BACKGROUND_COLOR);
const rgb_color selectedBackGround = tint_color(backGround, 1.2);
const rgb_color highlight = tint_color(backGround, B_LIGHTEN_MAX_TINT);
const rgb_color darken1 = tint_color(backGround, B_DARKEN_1_TINT);
const rgb_color darken2 = tint_color(backGround, B_DARKEN_2_TINT);
const rgb_color treeBackGround = tint_color(backGround, 1.1);;
const rgb_color treeSelectedBackGround = tint_color(treeBackGround, 1.2);
const rgb_color treeShadow = tint_color(treeBackGround, B_DARKEN_2_TINT);
const rgb_color treeSelectedShadow = tint_color(treeSelectedBackGround, B_DARKEN_2_TINT);
const rgb_color treeHighlight = tint_color(treeBackGround, B_LIGHTEN_MAX_TINT);
const rgb_color black = (rgb_color){ 0, 0, 0, 255 };

const column_tree_item_colors	kDefaultColumnTreeItemColors = {
	black,								// foreground
	treeBackGround,						// background
	treeHighlight,						// highlight
	treeShadow,							// shadow
	black,								// selected_foreground
	(rgb_color){						// selected_background
		treeSelectedBackGround.red,
		treeSelectedBackGround.green,
		treeSelectedBackGround.blue + (uint8)(treeSelectedBackGround.blue * BLUE_PERCENTAGE),
		treeSelectedBackGround.alpha
	},
	tint_color(treeSelectedBackGround, B_LIGHTEN_2_TINT),	// selected_highlight
	tint_color(treeSelectedBackGround, B_DARKEN_2_TINT)	// selected_shadow
};

const column_header_colors		kDefaultColumnHeaderColors = {
	black,								// foreground
	backGround,							// background
	highlight,							// highlight
	darken1,							// light_shadow
	darken2,							// shadow
	black,								// pressed_foreground
	selectedBackGround,					// pressed_background
	backGround,							// pressed_highlight
	treeSelectedShadow					// pressed_shadow
};

const column_header_view_colors	kDefaultColumnHeaderViewColors = {
	backGround,							// background
	kDefaultColumnHeaderColors			// header_colors
};

const column_tree_view_colors	kDefaultColumnTreeViewColors = {
	treeBackGround,						// background
	kDefaultColumnTreeItemColors,		// item_colors
	kDefaultColumnHeaderViewColors		// header_view_colors
};

