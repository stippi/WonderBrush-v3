#ifndef COLOR_PREVIEW_PLATFORM_DELEGATE_H
#define COLOR_PREVIEW_PLATFORM_DELEGATE_H


#include "ColorPreview.h"


class ColorPreview::PlatformDelegate {
public:
	PlatformDelegate(ColorPreview* view)
		:
		fView(view)
	{
		fView->SetViewColor(B_TRANSPARENT_COLOR);
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
