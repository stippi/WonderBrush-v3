#ifndef COLOR_PREVIEW_PLATFORM_DELEGATE_H
#define COLOR_PREVIEW_PLATFORM_DELEGATE_H


#include "ColorPreview.h"


class ColorPreview::PlatformDelegate {
public:
	PlatformDelegate(ColorPreview* view)
		:
		fView(view)
	{
		BSize size = fView->Bounds().Size();
		fView->SetExplicitMinSize(size);
	}

	void FillRect(PlatformDrawContext& drawContext, BRect rect,
		const rgb_color& color)
	{
		drawContext.Painter().fillRect(rect.ToQRect(), color);
	}

private:
	ColorPreview*	fView;
};


#endif // COLOR_PREVIEW_PLATFORM_DELEGATE_H
