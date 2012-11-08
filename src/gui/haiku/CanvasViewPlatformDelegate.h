#ifndef CANVAS_VIEW_PLATFORM_DELEGATE_H
#define CANVAS_VIEW_PLATFORM_DELEGATE_H


#include <Bitmap.h>
#include <Region.h>

#include "CanvasView.h"
#include "ui_defines.h"


class CanvasView::PlatformDelegate {
public:
	PlatformDelegate(CanvasView* view)
		:
		fView(view)
	{
		fView->SetViewColor(B_TRANSPARENT_32_BIT);
		fView->SetHighColor(kStripesHigh);
		fView->SetLowColor(kStripesLow);
			// used for drawing the stripes pattern
	}

	void DrawCanvas(PlatformDrawContext& drawContext, const BBitmap* bitmap,
		const BRect& canvas)
	{
		BView* view = drawContext.View();
		if (bitmap != NULL)
			view->DrawBitmap(bitmap, bitmap->Bounds(), canvas);
		else
			view->FillRect(canvas);
	}

	void DrawStripes(PlatformDrawContext& drawContext, const BRect& canvas)
	{
		BView* view = drawContext.View();
		BRegion outside(view->Bounds() & drawContext.UpdateRect());
		outside.Exclude(canvas);
		view->FillRegion(&outside, kStripes);
	}

	void ScrollBy(const BPoint& offset)
	{
#if CANVAS_VIEW_USE_NATIVE_SCROLLING
		fView->ScrollBy(offset.x, offset.y);
#else
		fView->Invalidate();
#endif
	}

private:
	CanvasView*	fView;
};


#endif // CANVAS_VIEW_PLATFORM_DELEGATE_H
