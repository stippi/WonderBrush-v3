#include "NavigatorViewPlatformDelegate.h"

#include <Bitmap.h>
#include <Region.h>

#include "platform_support_ui.h"
#include "ui_defines.h"


NavigatorView::PlatformDelegate::PlatformDelegate(NavigatorView* view)
	:
	fView(view),
	fStripesBrush(pattern_to_brush(kStripes, kStripesLow, kStripesHigh)),
	fDottedBrush(pattern_to_brush(kDotted, make_color(0, 0, 0),
		make_color(255, 255, 255, 170)))
{
}


void
NavigatorView::PlatformDelegate::DrawBitmap(PlatformDrawContext& drawContext,
	const BBitmap* bitmap, const BRect& iconBounds)
{
	if (QImage* image = bitmap->GetQImage()) {
		drawContext.Painter().drawImage(iconBounds.ToQRect(), *image,
			image->rect());
	}
}


void
NavigatorView::PlatformDelegate::DrawBackground(
	PlatformDrawContext& drawContext, const BRegion& region)
{
	QPainter& painter = drawContext.Painter();
	QRegion oldClipRegion = painter.clipRegion();
	QRegion qRegion = region.ToQRegion();
	painter.setClipRegion(qRegion);
	painter.fillRect(qRegion.boundingRect(), fStripesBrush);
	painter.setClipRegion(oldClipRegion);
}


void
NavigatorView::PlatformDelegate::DrawRect(PlatformDrawContext& drawContext,
	const BRect& visibleRect, const BRect& iconBounds)
{
	QPainter& painter = drawContext.Painter();
	QRegion oldClipRegion = painter.clipRegion();
		// TODO: Not sure, if resetting the clipping region is cheaper than
		// creating a new painter (also int DrawBackground()).

	QRegion visibleRectRegion(visibleRect.ToQRect());
	visibleRectRegion -= visibleRect.InsetByCopy(2, 2).ToQRect();
	painter.setClipRegion(visibleRectRegion);
	painter.fillRect(visibleRectRegion.boundingRect(), fDottedBrush);

	QRegion outside(iconBounds.ToQRect());
	outside -= visibleRect.InsetByCopy(1, 1).ToQRect();
	if (!outside.isEmpty()) {
		painter.setClipRegion(outside);
		painter.fillRect(outside.boundingRect(), QColor(0, 0, 0, 50));
	}

	painter.setClipRegion(oldClipRegion);
}
