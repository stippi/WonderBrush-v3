#include "platform_support_ui.h"

#include <InterfaceDefs.h>

#include "PlatformViewMixin.h"
#include "support_ui.h"


QBrush
pattern_to_brush(const struct pattern& pattern, const rgb_color& lowColor,
	const rgb_color& highColor)
{
	uint32 low = (uint32)lowColor.alpha << 24 | (uint32)lowColor.red << 16
		| (uint32)lowColor.green << 8 | (uint32)lowColor.blue;
	uint32 high = (uint32)highColor.alpha << 24 | (uint32)highColor.red << 16
		| (uint32)highColor.green << 8 | (uint32)highColor.blue;

	QImage texture = QImage(8, 8, QImage::Format_ARGB32);

	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			texture.setPixel(x, y,
				(pattern.data[y] & (1 << (7 - x))) != 0 ? high : low);
		}
	}

	return texture;
}


void
platform_draw_recessed_frame(PlatformDrawContext& drawContext,
	const BRect& rect, const rgb_color& baseColor)
{
	rgb_color background = baseColor;
	rgb_color shadow = tint_color(background, B_DARKEN_1_TINT);
	rgb_color darkShadow = tint_color(background, B_DARKEN_3_TINT);
	rgb_color light = tint_color(background, B_LIGHTEN_MAX_TINT);

	BRect r(rect);
	stroke_frame(drawContext, r, shadow, shadow, light, light);
	r.InsetBy(1, 1);
	stroke_frame(drawContext, r, darkShadow, darkShadow, background,
		background);
}


void
platform_draw_control_widget_frame(PlatformDrawContext& drawContext,
	BRect& rect, bool isEnabled, bool hasFocus)
{
// TODO: Support isEnabled and hasFocus!
	platform_draw_recessed_frame(drawContext, rect,
		ui_color(B_PANEL_BACKGROUND_COLOR));
	rect.InsetBy(2, 2);
}


// stroke_frame
void
stroke_frame(PlatformDrawContext& drawContext, BRect r, rgb_color left,
	rgb_color top, rgb_color right, rgb_color bottom)
{
	if (!r.IsValid())
		return;

	QPainter& painter = drawContext.Painter();

	painter.setPen(left);
	painter.drawLine(QPoint((int)r.left, r.bottom), QPoint(r.left, r.top));

	painter.setPen(top);
	painter.drawLine(QPoint((int)r.left + 1, (int)r.top),
		QPoint((int)r.right, (int)r.top));

	painter.setPen(right);
	painter.drawLine(QPoint((int)r.right, (int)r.top + 1),
		QPoint((int)r.right, (int)r.bottom));

	painter.setPen(bottom);
	painter.drawLine(QPoint((int)r.right - 1, (int)r.bottom),
		QPoint((int)r.left + 1, (int)r.bottom));
}
