#ifndef COLOR_SLIDER_PLATFORM_DELEGATE_H
#define COLOR_SLIDER_PLATFORM_DELEGATE_H


#include "ColorSlider.h"

#include <Bitmap.h>

#include <QPolygon>

#include "platform_support_ui.h"


class ColorSlider::PlatformDelegate {
public:
	PlatformDelegate(ColorSlider* view)
		:
		fView(view)
	{
	}

	void DrawBackground(PlatformDrawContext& drawContext, BRect bounds,
		BBitmap* bitmap, border_style borderStyle)
	{
		QPainter& painter = drawContext.Painter();

		// Frame
		if (borderStyle == B_FANCY_BORDER) {
			platform_draw_control_widget_frame(drawContext, bounds,
				fView->isEnabled(), fView->hasFocus());
		}

		// Color slider fill
		if (bitmap != NULL) {
			if (QImage* image = bitmap->GetQImage())
				painter.drawImage(bounds.LeftTop().ToQPoint(), *image);
		} else
			painter.fillRect(bounds.ToQRect(), QColor(255, 0, 0));
	}

	void DrawLine(PlatformDrawContext& drawContext, const BPoint& from,
		const BPoint& to, const rgb_color& color)
	{
		QPainter& painter = drawContext.Painter();
		painter.setPen(color);
		painter.drawLine(from.ToQPoint(), to.ToQPoint());
	}

	void DrawTriangle(PlatformDrawContext& drawContext, const BPoint& point1,
		const BPoint& point2, const BPoint& point3, const rgb_color& color)
	{
		QPainter& painter = drawContext.Painter();
		painter.setPen(color);
		painter.setBrush(Qt::NoBrush);
		painter.drawPolygon(QPolygon()
			<< point1.ToQPoint() << point2.ToQPoint() << point3.ToQPoint());
	}

private:
	ColorSlider*	fView;
};


#endif // COLOR_SLIDER_PLATFORM_DELEGATE_H
