#ifndef PICK_TOOL_STATE_PLATFORM_DELEGATE_H
#define PICK_TOOL_STATE_PLATFORM_DELEGATE_H


#include "PickToolState.h"

#include <QPainter>


class PickToolState::PlatformDelegate {
public:
	PlatformDelegate(PickToolState* state)
		:
		fState(state)
	{
	}

	void DrawRect(PlatformDrawContext& drawContext, BRect rect)
	{
#if 0
		BView* view = drawContext.View();
		view->SetHighColor(0, 0, 0);
		view->StrokeRect(rect);
		rect.InsetBy(-1, -1);
		view->SetHighColor(255, 255, 255);
		view->StrokeRect(rect);
#endif

		QPainter& painter = drawContext.Painter();
		painter.setPen(QColor(0, 0, 0));
		painter.drawRect(rect.ToQRect());
		rect.InsetBy(-1, -1);
		painter.setPen(QColor(255, 255, 255));
		painter.drawRect(rect.ToQRect());
	}

private:
	PickToolState*	fState;
};


#endif // PICK_TOOL_STATE_PLATFORM_DELEGATE_H
