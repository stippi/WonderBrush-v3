#ifndef PLATFORM_QT_SUPPORT_UI_H
#define PLATFORM_QT_SUPPORT_UI_H


#include <GraphicsDefs.h>

#include <QBrush>


QBrush pattern_to_brush(const struct pattern& pattern,
	const rgb_color& lowColor, const rgb_color& highColor);


#endif // PLATFORM_QT_SUPPORT_UI_H
