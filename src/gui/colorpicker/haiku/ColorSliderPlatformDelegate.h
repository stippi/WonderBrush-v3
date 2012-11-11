/*
 * Copyright 2001 Werner Freytag - please read to the LICENSE file
 *
 * Copyright 2002-2006, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 *
 */
#ifndef COLOR_SLIDER_PLATFORM_DELEGATE_H
#define COLOR_SLIDER_PLATFORM_DELEGATE_H


#include <ControlLook.h>

#include "ColorSlider.h"


class ColorSlider::PlatformDelegate {
public:
	PlatformDelegate(ColorSlider* view)
		:
		fView(view)
	{
		fView->SetViewColor(B_TRANSPARENT_COLOR);
		fView->SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	}

	void DrawBackground(PlatformDrawContext& drawContext, BRect bounds,
		BBitmap* bitmap, border_style borderStyle)
	{
		BView* view = drawContext.View();

		// Frame
		if (borderStyle == B_FANCY_BORDER) {
			bounds.InsetBy(-2, -2);
			rgb_color color = view->LowColor();
			be_control_look->DrawTextControlBorder(view, bounds,
				drawContext.UpdateRect(), color);
		}

		// Color slider fill
		if (bitmap != NULL)
			view->DrawBitmap(bitmap, bounds.LeftTop());
		else {
			view->SetHighColor(255, 0, 0);
			view->FillRect(bounds);
		}
	}

	void DrawLine(PlatformDrawContext& drawContext, const BPoint& from,
		const BPoint& to, const rgb_color& color)
	{
		BView* view = drawContext.View();
		view->SetHighColor(color);
		view->StrokeLine(from, to);
	}

	void DrawTriangle(PlatformDrawContext& drawContext, const BPoint& point1,
		const BPoint& point2, const BPoint& point3, const rgb_color& color)
	{
		BView* view = drawContext.View();
		view->SetHighColor(color);
		view->StrokeLine(point1, point2);
		view->StrokeLine(point3);
		view->StrokeLine(point1);
	}

private:
	ColorSlider*	fView;
};


#endif // COLOR_SLIDER_PLATFORM_DELEGATE_H
