#ifndef CANVAS_VIEW_PLATFORM_DELEGATE_H
#define CANVAS_VIEW_PLATFORM_DELEGATE_H


#include <Bitmap.h>

#include "CanvasView.h"
#include "platform_support_ui.h"
#include "ui_defines.h"


class CanvasView::PlatformDelegate {
public:
	PlatformDelegate(CanvasView* view)
		:
		fView(view),
		fStripesBrush(pattern_to_brush(kStripes, kStripesLow, kStripesHigh))
	{
	}

	void DrawCanvas(PlatformDrawContext& drawContext, const BBitmap* bitmap,
		const BRect& canvas)
	{
		QPainter& painter = drawContext.Painter();
		if (bitmap != NULL) {
			if (bitmap->GetQImage() != NULL) {
				painter.drawImage(canvas.ToQRect(), * bitmap->GetQImage(),
					bitmap->Bounds().ToQRect());
			}
		} else
			painter.fillRect(canvas.ToQRect(), kStripesHigh);
	}

	void DrawStripes(PlatformDrawContext& drawContext, const BRect& canvas)
	{
		QPainter& painter = drawContext.Painter();
		QRegion outside
			= drawContext.PaintEvent()->region().subtracted(canvas.ToQRect());
		if (!outside.isEmpty()) {
			painter.setClipRegion(outside);
			painter.fillRect(outside.boundingRect(), fStripesBrush);
		}
	}

	void ScrollBy(const BPoint& offset)
	{
		fView->scroll(offset.x, offset.y);
	}

private:
	CanvasView*	fView;
	QBrush		fStripesBrush;
};


#endif // CANVAS_VIEW_PLATFORM_DELEGATE_H
