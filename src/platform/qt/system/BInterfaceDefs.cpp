#include <InterfaceDefs.h>

#include <View.h>

#include <QApplication>
#include <QStyle>


uint32
modifiers()
{
	return BView::FromQtModifiers(QApplication::keyboardModifiers());
}


rgb_color
ui_color(color_which which)
{
	QPalette::ColorRole colorRole;
	switch (which) {
		case B_PANEL_BACKGROUND_COLOR:
			colorRole = QPalette::Window;
			break;
		case B_PANEL_TEXT_COLOR:
			colorRole = QPalette::WindowText;
			break;
		case B_DOCUMENT_BACKGROUND_COLOR:
			colorRole = QPalette::Base;
			break;
		case B_DOCUMENT_TEXT_COLOR:
			colorRole = QPalette::Text;
			break;
//		case B_CONTROL_BACKGROUND_COLOR:
//		case B_CONTROL_TEXT_COLOR:
//		case B_CONTROL_BORDER_COLOR:
//		case B_CONTROL_HIGHLIGHT_COLOR:
//		case B_CONTROL_MARK_COLOR:
//		case B_NAVIGATION_BASE_COLOR:
//		case B_NAVIGATION_PULSE_COLOR:
//		case B_SHINE_COLOR:
//		case B_SHADOW_COLOR:
//		case B_MENU_BACKGROUND_COLOR:
//		case B_MENU_SELECTED_BACKGROUND_COLOR:
//		case B_MENU_ITEM_TEXT_COLOR:
//		case B_MENU_SELECTED_ITEM_TEXT_COLOR:
//		case B_MENU_SELECTED_BORDER_COLOR:
//		case B_LIST_BACKGROUND_COLOR:
//		case B_LIST_SELECTED_BACKGROUND_COLOR:
//		case B_LIST_ITEM_TEXT_COLOR:
//		case B_LIST_SELECTED_ITEM_TEXT_COLOR:
//		case B_TOOL_TIP_BACKGROUND_COLOR:
//		case B_TOOL_TIP_TEXT_COLOR:
//		case B_SUCCESS_COLOR:
//		case B_FAILURE_COLOR:
//		case B_WINDOW_TAB_COLOR:
//		case B_WINDOW_TEXT_COLOR:
//		case B_WINDOW_INACTIVE_TAB_COLOR:
//		case B_WINDOW_INACTIVE_TEXT_COLOR:
//		case B_WINDOW_BORDER_COLOR:
//		case B_WINDOW_INACTIVE_BORDER_COLOR:
//		case B_DESKTOP_COLOR:
		default:
			debugger("ui_color(): Unsupported color!");
			return make_color(0, 0, 0);
	}

	return rgb_color::FromQColor(
		QApplication::style()->standardPalette().color(colorRole));
}


rgb_color
tint_color(rgb_color color, float tint)
{
	rgb_color result;

	#define LIGHTEN(x) ((uint8)(255.0f - (255.0f - x) * tint))
	#define DARKEN(x)  ((uint8)(x * (2 - tint)))

	if (tint < 1.0f) {
		result.red   = LIGHTEN(color.red);
		result.green = LIGHTEN(color.green);
		result.blue  = LIGHTEN(color.blue);
		result.alpha = color.alpha;
	} else {
		result.red   = DARKEN(color.red);
		result.green = DARKEN(color.green);
		result.blue  = DARKEN(color.blue);
		result.alpha = color.alpha;
	}

	#undef LIGHTEN
	#undef DARKEN

	return result;
}
