#ifndef RECTANGLE_TOOL_STATE_PLATFORM_DELEGATE_H
#define RECTANGLE_TOOL_STATE_PLATFORM_DELEGATE_H


#include <Shape.h>

#include "RectangleToolState.h"
#include "ui_defines.h"


class RectangleToolState::PlatformDelegate {
public:
	PlatformDelegate(RectangleToolState* state)
		:
		fState(state)
	{
	}

	void DrawRectangle(PlatformDrawContext& drawContext,
		TransformViewState& viewState, float zoomLevel)
	{
	}

private:
	RectangleToolState*	fState;
};


#endif // RECTANGLE_TOOL_STATE_PLATFORM_DELEGATE_H
