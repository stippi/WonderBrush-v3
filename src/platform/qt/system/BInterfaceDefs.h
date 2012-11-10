/*
 * Copyright 2001-2008, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef INTERFACE_DEFS_H
#define INTERFACE_DEFS_H


#include <GraphicsDefs.h>
#include <OS.h>


class BBitmap;
class BPoint;
class BRect;


// Key definitions

struct key_info {
	uint32	modifiers;
	uint8	key_states[16];
};

enum {
	B_BACKSPACE			= 0x08,
	B_RETURN			= 0x0a,
	B_ENTER				= 0x0a,
	B_SPACE				= 0x20,
	B_TAB				= 0x09,
	B_ESCAPE			= 0x1b,
	B_SUBSTITUTE		= 0x1a,

	B_LEFT_ARROW		= 0x1c,
	B_RIGHT_ARROW		= 0x1d,
	B_UP_ARROW			= 0x1e,
	B_DOWN_ARROW		= 0x1f,

	B_INSERT			= 0x05,
	B_DELETE			= 0x7f,
	B_HOME				= 0x01,
	B_END				= 0x04,
	B_PAGE_UP			= 0x0b,
	B_PAGE_DOWN			= 0x0c,

	B_FUNCTION_KEY		= 0x10,

	// for Japanese keyboards
	B_KATAKANA_HIRAGANA	= 0xf2,
	B_HANKAKU_ZENKAKU	= 0xf3
};

enum {
	B_F1_KEY			= 0x02,
	B_F2_KEY			= 0x03,
	B_F3_KEY			= 0x04,
	B_F4_KEY			= 0x05,
	B_F5_KEY			= 0x06,
	B_F6_KEY			= 0x07,
	B_F7_KEY			= 0x08,
	B_F8_KEY			= 0x09,
	B_F9_KEY			= 0x0a,
	B_F10_KEY			= 0x0b,
	B_F11_KEY			= 0x0c,
	B_F12_KEY			= 0x0d,
	B_PRINT_KEY			= 0x0e,
	B_SCROLL_KEY		= 0x0f,
	B_PAUSE_KEY			= 0x10
};


// modifiers
enum {
	B_SHIFT_KEY			= 0x00000001,
	B_COMMAND_KEY		= 0x00000002,
	B_CONTROL_KEY		= 0x00000004,
	B_CAPS_LOCK			= 0x00000008,
	B_SCROLL_LOCK		= 0x00000010,
	B_NUM_LOCK			= 0x00000020,
	B_OPTION_KEY		= 0x00000040,
	B_MENU_KEY			= 0x00000080,
	B_LEFT_SHIFT_KEY	= 0x00000100,
	B_RIGHT_SHIFT_KEY	= 0x00000200,
	B_LEFT_COMMAND_KEY	= 0x00000400,
	B_RIGHT_COMMAND_KEY	= 0x00000800,
	B_LEFT_CONTROL_KEY	= 0x00001000,
	B_RIGHT_CONTROL_KEY	= 0x00002000,
	B_LEFT_OPTION_KEY	= 0x00004000,
	B_RIGHT_OPTION_KEY	= 0x00008000
};


// View orientation/alignment/style

enum border_style {
	B_PLAIN_BORDER,
	B_FANCY_BORDER,
	B_NO_BORDER
};

enum orientation {
	B_HORIZONTAL,
	B_VERTICAL
};


enum alignment {
	B_ALIGN_LEFT,
	B_ALIGN_RIGHT,
	B_ALIGN_CENTER,

	B_ALIGN_HORIZONTAL_CENTER	= B_ALIGN_CENTER,

	B_ALIGN_HORIZONTAL_UNSET	= -1L,
	B_ALIGN_USE_FULL_WIDTH		= -2L
};

enum vertical_alignment {
	B_ALIGN_TOP					= 0x10L,
	B_ALIGN_MIDDLE				= 0x20,
	B_ALIGN_BOTTOM				= 0x30,

	B_ALIGN_VERTICAL_CENTER		= B_ALIGN_MIDDLE,

	B_ALIGN_VERTICAL_UNSET		= -1L,
	B_ALIGN_NO_VERTICAL			= B_ALIGN_VERTICAL_UNSET,
	B_ALIGN_USE_FULL_HEIGHT		= -2L
};


// Default UI Colors

enum color_which {
	B_PANEL_BACKGROUND_COLOR = 1,
	B_PANEL_TEXT_COLOR = 10,
	B_DOCUMENT_BACKGROUND_COLOR = 11,
	B_DOCUMENT_TEXT_COLOR = 12,
	B_CONTROL_BACKGROUND_COLOR = 13,
	B_CONTROL_TEXT_COLOR = 14,
	B_CONTROL_BORDER_COLOR = 15,
	B_CONTROL_HIGHLIGHT_COLOR = 16,
	B_CONTROL_MARK_COLOR = 27,
	B_NAVIGATION_BASE_COLOR = 4,
	B_NAVIGATION_PULSE_COLOR = 17,
	B_SHINE_COLOR = 18,
	B_SHADOW_COLOR = 19,

	B_MENU_BACKGROUND_COLOR = 2,
	B_MENU_SELECTED_BACKGROUND_COLOR = 6,
	B_MENU_ITEM_TEXT_COLOR = 7,
	B_MENU_SELECTED_ITEM_TEXT_COLOR = 8,
	B_MENU_SELECTED_BORDER_COLOR = 9,

	B_LIST_BACKGROUND_COLOR = 28,
	B_LIST_SELECTED_BACKGROUND_COLOR = 29,
	B_LIST_ITEM_TEXT_COLOR = 30,
	B_LIST_SELECTED_ITEM_TEXT_COLOR = 31,

	B_TOOL_TIP_BACKGROUND_COLOR = 20,
	B_TOOL_TIP_TEXT_COLOR = 21,

	B_SUCCESS_COLOR = 100,
	B_FAILURE_COLOR = 101,

	B_WINDOW_TAB_COLOR = 3,
	B_WINDOW_TEXT_COLOR = 22,
	B_WINDOW_INACTIVE_TAB_COLOR = 23,
	B_WINDOW_INACTIVE_TEXT_COLOR = 24,

	B_WINDOW_BORDER_COLOR = 25,
	B_WINDOW_INACTIVE_BORDER_COLOR = 26,

	// Old name synonyms.
	B_KEYBOARD_NAVIGATION_COLOR = B_NAVIGATION_BASE_COLOR,
	B_MENU_SELECTION_BACKGROUND_COLOR = B_MENU_SELECTED_BACKGROUND_COLOR,

	// These are deprecated -- do not use in new code.  See BScreen for
	// the replacement for B_DESKTOP_COLOR.
	B_DESKTOP_COLOR = 5
};


// Color tinting

const float B_LIGHTEN_MAX_TINT	= 0.0f;		// 216 --> 255.0 (255)
const float B_LIGHTEN_2_TINT	= 0.385f;	// 216 --> 240.0 (240)
const float B_LIGHTEN_1_TINT	= 0.590f;	// 216 --> 232.0 (232)

const float B_NO_TINT			= 1.0f;		// 216 --> 216.0 (216)

const float B_DARKEN_1_TINT		= 1.147f;	// 216 --> 184.2 (184)
const float B_DARKEN_2_TINT		= 1.295f;	// 216 --> 152.3 (152)
const float B_DARKEN_3_TINT		= 1.407f;	// 216 --> 128.1 (128)
const float B_DARKEN_4_TINT		= 1.555f;	// 216 -->  96.1  (96)
const float B_DARKEN_MAX_TINT	= 2.0f;		// 216 -->   0.0   (0)
											// effects on standard gray level

const float B_DISABLED_LABEL_TINT		= B_DARKEN_3_TINT;
const float B_HIGHLIGHT_BACKGROUND_TINT	= B_DARKEN_2_TINT;
const float B_DISABLED_MARK_TINT		= B_LIGHTEN_2_TINT;


uint32			modifiers();

rgb_color		ui_color(color_which which);
rgb_color		tint_color(rgb_color color, float tint);


#endif // INTERFACE_DEFS_H
