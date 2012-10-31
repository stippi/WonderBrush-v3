#include "NavigatorViewPlatformDelegate.h"

#include <Bitmap.h>
#include <Region.h>

#include "platform_support_ui.h"
#include "ui_defines.h"


NavigatorView::PlatformDelegate::PlatformDelegate(NavigatorView* view)
	:
	fView(view)
{
	fView->SetViewColor(B_TRANSPARENT_COLOR);
	fView->SetHighColor(kStripesHigh);
	fView->SetLowColor(kStripesLow);
		// used for drawing the stripes pattern
}


void
NavigatorView::PlatformDelegate::DrawBitmap(PlatformDrawContext& drawContext,
	const BBitmap* bitmap, const BRect& iconBounds)
{
	fView->DrawBitmapAsync(bitmap, bitmap->Bounds(), iconBounds,
		B_FILTER_BITMAP_BILINEAR);
}


void
NavigatorView::PlatformDelegate::DrawBackground(
	PlatformDrawContext& drawContext, const BRegion& region)
{
	fView->FillRegion(&region, kStripes);
}


void
NavigatorView::PlatformDelegate::DrawRect(PlatformDrawContext& drawContext,
	const BRect& _visibleRect, const BRect& iconBounds)
{
	BRect visibleRect(_visibleRect);
	fView->SetHighColor(255, 255, 255, 170);
	fView->SetLowColor(0, 0, 0);
	fView->SetDrawingMode(B_OP_ALPHA);
	fView->SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_OVERLAY);
	fView->StrokeRect(visibleRect, kDotted);
	visibleRect.InsetBy(1, 1);
	fView->StrokeRect(visibleRect, kDotted);

	outside.Set(iconBounds);
	outside.Exclude(visibleRect);
	fView->SetHighColor(0, 0, 0, 50);
	fView->FillRegion(&outside);
}
