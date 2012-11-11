/*
 * Copyright 2006-2012 Stephan Aßmus <superstippi@gmx.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef ALPHA_SLIDER_PLATFORM_DELEGATE_H
#define ALPHA_SLIDER_PLATFORM_DELEGATE_H


#include <Bitmap.h>

#include "AlphaSlider.h"

#include "platform_support_ui.h"
#include "ui_defines.h"


class AlphaSlider::PlatformDelegate {
public:
	PlatformDelegate(AlphaSlider* view)
		:
		fView(view)
	{
	}

	void DrawBackground(PlatformDrawContext& drawContext, BRect bounds,
		border_style borderStyle, BBitmap* bitmap, rgb_color backgroundColor,
		bool isEnabled, bool hasFocus)
	{
		QPainter& painter = drawContext.Painter();

		if (borderStyle == B_FANCY_BORDER) {
			platform_draw_control_widget_frame(drawContext, bounds, isEnabled,
				hasFocus);
		}

		if (bitmap != NULL) {
			if (QImage* image = bitmap->GetQImage())
				painter.drawImage(bounds.LeftTop().ToQPoint(), *image);
		}
	}

	void DrawLine(PlatformDrawContext& drawContext, const BPoint& from,
		const BPoint& to, const rgb_color& color)
	{
		QPainter& painter = drawContext.Painter();
		painter.setPen(color);
		painter.drawLine(from.ToQPoint(), to.ToQPoint());
	}

	rgb_color BackgroundColor() const
	{
		return rgb_color::FromQColor(
			fView->palette().color(fView->backgroundRole()));
	}

private:
	AlphaSlider*	fView;
};


#endif // ALPHA_SLIDER_PLATFORM_DELEGATE_H
