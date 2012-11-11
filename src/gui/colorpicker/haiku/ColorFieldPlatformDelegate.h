/*
 * Copyright 2001 Werner Freytag - please read to the LICENSE file
 *
 * Copyright 2002-2006, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 *
 */
#ifndef COLOR_FIELD_PLATFORM_DELEGATE_H
#define COLOR_FIELD_PLATFORM_DELEGATE_H


#include <ControlLook.h>

#include "ColorField.h"


class ColorField::PlatformDelegate {
public:
	PlatformDelegate(ColorField* view)
		:
		fView(view)
	{
		fView->SetViewColor(B_TRANSPARENT_COLOR);
		fView->SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	}

	void Draw(PlatformDrawContext& drawContext, BRect bounds,
		border_style borderStyle, BBitmap* bitmap, const BPoint& markerPosition)
	{
		BView* view = drawContext.View();

		// Frame
		if (borderStyle == B_FANCY_BORDER) {
			rgb_color color = view->LowColor();
			be_control_look->DrawTextControlBorder(view, bounds,
				drawContext.UpdateRect(), color);
		}

		// Color field fill
		if (bitmap != NULL)
			view->DrawBitmap(bitmap, bounds.LeftTop());
		else {
			view->SetHighColor(255, 0, 0);
			view->FillRect(bounds);
		}

		// Marker
		view->SetHighColor(0, 0, 0);
		view->StrokeEllipse(markerPosition + bounds.LeftTop(), 5.0, 5.0);
		view->SetHighColor(255.0, 255.0, 255.0);
		view->StrokeEllipse(markerPosition + bounds.LeftTop(), 4.0, 4.0);
	}

private:
	ColorField*	fView;
};


#endif // COLOR_FIELD_PLATFORM_DELEGATE_H
