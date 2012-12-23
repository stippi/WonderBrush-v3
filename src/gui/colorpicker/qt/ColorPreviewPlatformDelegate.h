#ifndef COLOR_PREVIEW_PLATFORM_DELEGATE_H
#define COLOR_PREVIEW_PLATFORM_DELEGATE_H


#include "ColorPreview.h"

#include "platform_support_ui.h"


class ColorPreview::PlatformDelegate {
public:
	PlatformDelegate(ColorPreview* view)
		:
		fView(view)
	{
		BSize size = fView->Bounds().Size();
		fView->SetExplicitMinSize(size);
	}

	void DrawBackground(PlatformDrawContext& drawContext, BRect& bounds,
		border_style borderStyle)
	{
		// Frame
		if (borderStyle == B_FANCY_BORDER) {
			platform_draw_control_widget_frame(drawContext, bounds,
				fView->isEnabled(), fView->hasFocus());
		}
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
