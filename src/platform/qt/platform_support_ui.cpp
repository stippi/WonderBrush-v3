#include "platform_support_ui.h"


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

