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

	void DrawBox(PlatformDrawContext& drawContext,
		const BPoint& lt, const BPoint& rt, const BPoint& rb,
		const BPoint& lb, float zoomLevel)
	{
		BView* view = drawContext.View();
		
		view->SetDrawingMode(B_OP_ALPHA);
		
		view->SetHighColor(0, 0, 0, 100);
		view->SetPenSize(3.0f);

		BShape shape;
		shape.MoveTo(lt);
		shape.LineTo(rt);
		shape.LineTo(rb);
		shape.LineTo(lb);
		shape.Close();
		
		view->MovePenTo(B_ORIGIN);
		view->StrokeShape(&shape);

		view->SetHighColor(255, 255, 255, 220);
		view->SetPenSize(1.0f);
		
		view->StrokeShape(&shape);
		
		DrawCorner(view, lt);
		DrawCorner(view, rt);
		DrawCorner(view, rb);
		DrawCorner(view, lb);
	}
	
	void DrawCorner(BView* view, const BPoint& corner)
	{
		view->SetHighColor(0, 0, 0, 100);

		BRect rect(corner.x, corner.y, corner.x, corner.y);
		rect.InsetBy(-3, -3);
		view->FillEllipse(rect);

		view->SetHighColor(255, 255, 255, 255);
		rect.InsetBy(1, 1);
		view->FillEllipse(rect);
	}

private:
	RectangleToolState*	fState;
};


#endif // RECTANGLE_TOOL_STATE_PLATFORM_DELEGATE_H
