#ifndef PICK_TOOL_STATE_PLATFORM_DELEGATE_H
#define PICK_TOOL_STATE_PLATFORM_DELEGATE_H


#include "PickToolState.h"


class PickToolState::PlatformDelegate {
public:
	PlatformDelegate(PickToolState* state)
		:
		fState(state)
	{
	}

	void DrawRect(PlatformDrawContext& drawContext, BRect rect)
	{
		BView* view = drawContext.View();
		view->SetHighColor(0, 0, 0);
		view->StrokeRect(rect);
		rect.InsetBy(-1, -1);
		view->SetHighColor(255, 255, 255);
		view->StrokeRect(rect);
	}

private:
	PickToolState*	fState;
};


#endif // PICK_TOOL_STATE_PLATFORM_DELEGATE_H
