#ifndef SWATCH_VIEW_PLATFORM_DELEGATE_H
#define SWATCH_VIEW_PLATFORM_DELEGATE_H


#include "SwatchView.h"

#include <QPolygon>

#include "platform_support_ui.h"
#include "ui_defines.h"


class SwatchView::PlatformDelegate {
public:
	PlatformDelegate(SwatchView* view)
		:
		fView(view)
	{
		BSize size = fView->Bounds().Size();
		fView->SetExplicitMinSize(size);
	}

	void ColorChanged(const rgb_color& color)
	{
	}

	void DrawDottedBorder(PlatformDrawContext& drawContext, const BRect& rect,
		const rgb_color& leftTopHighColor, const rgb_color& leftTopLowColor,
		const rgb_color& rightBottomHighColor,
		const rgb_color& rightBottomLowColor)
	{
		QPainter& painter = drawContext.Painter();
		painter.setPen(QPen(
			pattern_to_brush(kDottedBig, leftTopLowColor, leftTopHighColor),
			1));
		painter.drawPolyline(
			QPolygon()
				<< QPoint((int)rect.left, (int)rect.bottom - 1)
				<< QPoint((int)rect.left, (int)rect.top)
				<< QPoint((int)rect.right, (int)rect.top));

		painter.setPen(QPen(
			pattern_to_brush(kDottedBig, rightBottomLowColor,
				rightBottomHighColor),
			1));
		painter.drawPolyline(
			QPolygon()
				<< QPoint((int)rect.right, (int)rect.top + 1)
				<< QPoint((int)rect.right, (int)rect.bottom)
				<< QPoint((int)rect.left, (int)rect.bottom));
	}

	void DrawPlainBorder(PlatformDrawContext& drawContext, const BRect& rect,
		const rgb_color& leftTopColor, const rgb_color& rightBottomColor)
	{
		QPainter& painter = drawContext.Painter();
		painter.setPen(leftTopColor);
		painter.drawPolyline(
			QPolygon()
				<< QPoint((int)rect.left, (int)rect.bottom - 1)
				<< QPoint((int)rect.left, (int)rect.top)
				<< QPoint((int)rect.right, (int)rect.top));

		painter.setPen(rightBottomColor);
		painter.drawPolyline(
			QPolygon()
				<< QPoint((int)rect.right, (int)rect.top + 1)
				<< QPoint((int)rect.right, (int)rect.bottom)
				<< QPoint((int)rect.left, (int)rect.bottom));
	}

	void FillDottedRect(PlatformDrawContext& drawContext, const BRect& rect,
		const rgb_color& highColor, const rgb_color& lowColor)
	{
		QPainter& painter = drawContext.Painter();
		painter.fillRect(rect.ToQRect(),
			pattern_to_brush(kDottedBig, lowColor, highColor));
	}

	void FillPlainRect(PlatformDrawContext& drawContext, const BRect& rect,
		const rgb_color& color)
	{
		QPainter& painter = drawContext.Painter();
		painter.fillRect(rect.ToQRect(), color);
	}

private:
	SwatchView*	fView;
};


#endif // SWATCH_VIEW_PLATFORM_DELEGATE_H
