/*
 * Copyright 2006-2012 Stephan Aßmus <superstippi@gmx.de>
 * Distributed under the terms of the MIT License.
 */
#ifndef ALPHA_SLIDER_PLATFORM_DELEGATE_H
#define ALPHA_SLIDER_PLATFORM_DELEGATE_H


#include <ControlLook.h>

#include "AlphaSlider.h"

#include "ui_defines.h"


class AlphaSlider::PlatformDelegate {
public:
	PlatformDelegate(AlphaSlider* view)
		:
		fView(view)
	{
		fView->SetViewColor(B_TRANSPARENT_COLOR);
		fView->SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	}

	void DrawBackground(PlatformDrawContext& drawContext, BRect bounds,
		border_style borderStyle, BBitmap* bitmap, rgb_color backgroundColor,
		bool isEnabled, bool hasFocus)
	{
		BView* view = drawContext.View();

		if (borderStyle == B_FANCY_BORDER) {
			uint32 flags = 0;

			if (!isEnabled)
				flags |= BControlLook::B_DISABLED;
			if (hasFocus)
				flags |= BControlLook::B_FOCUSED;

			be_control_look->DrawTextControlBorder(view, bounds,
				drawContext.UpdateRect(), backgroundColor, flags);
		}

		view->DrawBitmap(bitmap, bounds.LeftTop());
	}

	void DrawLine(PlatformDrawContext& drawContext, const BPoint& from,
		const BPoint& to, const rgb_color& color)
	{
		BView* view = drawContext.View();
		view->SetHighColor(color);
		view->StrokeLine(from, to);
	}

	rgb_color BackgroundColor() const
	{
		return fView->LowColor();
	}

private:
	AlphaSlider*	fView;
};


#endif // ALPHA_SLIDER_PLATFORM_DELEGATE_H
