#ifndef COLOR_FIELD_PLATFORM_DELEGATE_H
#define COLOR_FIELD_PLATFORM_DELEGATE_H


#include <Bitmap.h>

#include "ColorField.h"
#include "platform_support_ui.h"


class ColorField::PlatformDelegate {
public:
	PlatformDelegate(ColorField* view)
		:
		fView(view)
	{
		BSize size = fView->Bounds().Size();
		fView->SetExplicitMinSize(size);
	}

	void Draw(PlatformDrawContext& drawContext, BRect bounds,
		border_style borderStyle, BBitmap* bitmap, const BPoint& markerPosition)
	{
		QPainter& painter = drawContext.Painter();

		// Frame
		if (borderStyle == B_FANCY_BORDER) {
			platform_draw_control_widget_frame(drawContext, bounds,
				fView->isEnabled(), fView->hasFocus());
		}

		// Color field fill
		if (bitmap != NULL) {
			if (QImage* image = bitmap->GetQImage())
				painter.drawImage(bounds.LeftTop().ToQPoint(), *image);
		} else
			painter.fillRect(bounds.ToQRect(), QColor(255, 0, 0));

		// Marker
		// Note: With antialiased drawing the circle looks a bit angled. Without
		// it doesn't look to good either.
//		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setBrush(Qt::NoBrush);
		painter.setPen(QColor(0, 0, 0));
		painter.drawEllipse(markerPosition + bounds.LeftTop(), 5.0, 5.0);
		painter.setPen(QColor(255, 255, 255));
		painter.drawEllipse(markerPosition + bounds.LeftTop(), 4.0, 4.0);
	}

private:
	ColorField*	fView;
};


#endif // COLOR_FIELD_PLATFORM_DELEGATE_H
