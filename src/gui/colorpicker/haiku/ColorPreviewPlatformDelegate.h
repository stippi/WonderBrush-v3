#ifndef COLOR_PREVIEW_PLATFORM_DELEGATE_H
#define COLOR_PREVIEW_PLATFORM_DELEGATE_H


#include "ColorPreview.h"

#include <ControlLook.h>


class ColorPreview::PlatformDelegate {
public:
	PlatformDelegate(ColorPreview* view)
		:
		fView(view)
	{
		fView->SetViewColor(B_TRANSPARENT_COLOR);
	}

	void DrawBackground(PlatformDrawContext& drawContext, BRect& bounds,
		border_style borderStyle)
	{
		// Frame
		if (borderStyle == B_FANCY_BORDER) {
			BView* view = drawContext.View();
			rgb_color color = view->LowColor();
			be_control_look->DrawTextControlBorder(view, bounds,
				drawContext.UpdateRect(), color);
		}
	}

	void FillRect(PlatformDrawContext& drawContext, BRect rect,
		const rgb_color& color)
	{
		BView* view = drawContext.View();
		view->SetHighColor(color);
		view->FillRect(rect);
	}

private:
	ColorPreview*	fView;
};


#endif // COLOR_PREVIEW_PLATFORM_DELEGATE_H
