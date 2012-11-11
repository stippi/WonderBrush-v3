/*
 * Copyright 2006-2012, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#ifndef SWATCH_VIEW_PLATFORM_DELEGATE_H
#define SWATCH_VIEW_PLATFORM_DELEGATE_H


#include "SwatchView.h"

#include "ui_defines.h"


class SwatchView::PlatformDelegate {
public:
	PlatformDelegate(SwatchView* view)
		:
		fView(view)
	{
		fView->SetViewColor(B_TRANSPARENT_COLOR);
		fView->SetHighColor(fView->fColor);
	}

	void ColorChanged(const rgb_color& color)
	{
		fView->SetHighColor(color);
	}

	void DrawDottedBorder(PlatformDrawContext& drawContext, const BRect& rect,
		const rgb_color& leftTopHighColor, const rgb_color& leftTopLowColor,
		const rgb_color& rightBottomHighColor,
		const rgb_color& rightBottomLowColor)
	{
		BView* view = drawContext.View();

		view->SetHighColor(leftTopHighColor);
		view->SetLowColor(leftTopLowColor);

		view->StrokeLine(BPoint(rect.left, rect.bottom - 1),
				   BPoint(rect.left, rect.top), kDottedBig);
		view->StrokeLine(BPoint(rect.left + 1, rect.top),
				   BPoint(rect.right, rect.top), kDottedBig);

		// right/bottom
		view->SetHighColor(rightBottomHighColor);
		view->SetLowColor(rightBottomLowColor);

		view->StrokeLine(BPoint(rect.right, rect.top + 1),
				   BPoint(rect.right, rect.bottom), kDottedBig);
		view->StrokeLine(BPoint(rect.right - 1, rect.bottom),
				   BPoint(rect.left, rect.bottom), kDottedBig);
	}

	void DrawPlainBorder(PlatformDrawContext& drawContext, const BRect& rect,
		const rgb_color& leftTopColor, const rgb_color& rightBottomColor)
	{
		BView* view = drawContext.View();

		view->BeginLineArray(4);
			view->AddLine(BPoint(rect.left, rect.bottom - 1),
					BPoint(rect.left, rect.top), leftTopColor);
			view->AddLine(BPoint(rect.left + 1, rect.top),
					BPoint(rect.right, rect.top), leftTopColor);
			view->AddLine(BPoint(rect.right, rect.top + 1),
					BPoint(rect.right, rect.bottom), rightBottomColor);
			view->AddLine(BPoint(rect.right - 1, rect.bottom),
					BPoint(rect.left, rect.bottom), rightBottomColor);
		view->EndLineArray();
	}

	void FillDottedRect(PlatformDrawContext& drawContext, const BRect& rect,
		const rgb_color& highColor, const rgb_color& lowColor)
	{
		BView* view = drawContext.View();

		view->SetHighColor(highColor);
		view->SetLowColor(lowColor);

		view->FillRect(rect, kDottedBig);
	}

	void FillPlainRect(PlatformDrawContext& drawContext, const BRect& rect,
		const rgb_color& color)
	{
		BView* view = drawContext.View();

		view->SetHighColor(color);
		view->FillRect(rect);
	}

private:
	SwatchView*	fView;
};


#endif // SWATCH_VIEW_PLATFORM_DELEGATE_H
