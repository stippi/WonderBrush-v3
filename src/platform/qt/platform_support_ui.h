#ifndef PLATFORM_QT_SUPPORT_UI_H
#define PLATFORM_QT_SUPPORT_UI_H


#include <GraphicsDefs.h>
#include <Rect.h>

#include <QBrush>


class PlatformDrawContext;


QBrush pattern_to_brush(const struct pattern& pattern,
	const rgb_color& lowColor, const rgb_color& highColor);

void platform_draw_recessed_frame(PlatformDrawContext& drawContext,
	const BRect& rect, const rgb_color& baseColor);
void platform_draw_control_widget_frame(PlatformDrawContext& drawContext,
	BRect& rect, bool isEnabled, bool hasFocus);


#endif // PLATFORM_QT_SUPPORT_UI_H
