#ifndef TRANSFORM_TOOL_STATE_PLATFORM_DELEGATE_H
#define TRANSFORM_TOOL_STATE_PLATFORM_DELEGATE_H


#include "TransformToolState.h"

#include <Shape.h>


class TransformToolState::PlatformDelegate {
public:
	PlatformDelegate(TransformToolState* state)
		:
		fState(state)
	{
	}

	void Draw(PlatformDrawContext& drawContext, DrawParameters& parameters)
	{
		BView* view = drawContext.View();

		uint32 flags = view->Flags();
		if (subpixelPrecise)
			view->SetFlags(flags | B_SUBPIXEL_PRECISE);
		else
			view->SetFlags(flags & ~B_SUBPIXEL_PRECISE);

		view->PushState();

		view->MovePenTo(B_ORIGIN);

		// white corner fills
		BShape shape;
		shape.MoveTo(parameters.ltF0);
		shape.LineTo(parameters.ltF1);
		shape.LineTo(parameters.ltF2);
		shape.LineTo(parameters.ltF3);
		shape.Close();

		shape.MoveTo(parameters.rtF0);
		shape.LineTo(parameters.rtF1);
		shape.LineTo(parameters.rtF2);
		shape.LineTo(parameters.rtF3);
		shape.Close();

		shape.MoveTo(parameters.rbF0);
		shape.LineTo(parameters.rbF1);
		shape.LineTo(parameters.rbF2);
		shape.LineTo(parameters.rbF3);
		shape.Close();

		shape.MoveTo(parameters.lbF0);
		shape.LineTo(parameters.lbF1);
		shape.LineTo(parameters.lbF2);
		shape.LineTo(parameters.lbF3);
		shape.Close();

		view->SetHighColor(255, 255, 255);
		view->FillShape(&shape);

		shape.Clear();

		// transparent side fills

		shape.MoveTo(parameters.ltF0);
		shape.LineTo(parameters.lt3);
		shape.LineTo(parameters.rt1);
		shape.LineTo(parameters.rtF0);
		shape.Close();

		shape.MoveTo(parameters.rtF0);
		shape.LineTo(parameters.rt3);
		shape.LineTo(parameters.rb1);
		shape.LineTo(parameters.rbF0);
		shape.Close();

		shape.MoveTo(parameters.rbF0);
		shape.LineTo(parameters.rb3);
		shape.LineTo(parameters.lb1);
		shape.LineTo(parameters.lbF0);
		shape.Close();

		shape.MoveTo(parameters.lbF0);
		shape.LineTo(parameters.lb3);
		shape.LineTo(parameters.lt1);
		shape.LineTo(parameters.ltF0);
		shape.Close();

		view->SetHighColor(0, 0, 0, 30);
		view->SetDrawingMode(B_OP_ALPHA);
		view->FillShape(&shape);

		// white outlines

		shape.Clear();

		shape.MoveTo(parameters.ltF0);
		shape.LineTo(parameters.rtF0);
		shape.LineTo(parameters.rbF0);
		shape.LineTo(parameters.lbF0);
		shape.Close();

		view->SetHighColor(255, 255, 255, 200);
		view->StrokeShape(&shape);

		// black outlines

		shape.Clear();

		shape.MoveTo(parameters.lt1);
		shape.LineTo(parameters.rt3);
		shape.LineTo(parameters.rt2);
		shape.LineTo(parameters.rt1);
		shape.LineTo(parameters.rb3);
		shape.LineTo(parameters.rb2);
		shape.LineTo(parameters.rb1);
		shape.LineTo(parameters.lb3);
		shape.LineTo(parameters.lb2);
		shape.LineTo(parameters.lb1);
		shape.LineTo(parameters.lt3);
		shape.LineTo(parameters.lt2);
		shape.Close();

		view->SetHighColor(0, 0, 0, 200);
//		view->SetDrawingMode(B_OP_ALPHA);
		view->StrokeShape(&shape);

		// pivot
		view->SetHighColor(255, 255, 255, 200);
		view->FillEllipse(BRect(parameters.pivot.x - parameters.pivotSize,
			parameters.pivot.y - parameters.pivotSize,
			parameters.pivot.x + parameters.pivotSize + 0.5,
			parameters.pivot.y + parameters.pivotSize + 0.5));
		view->SetHighColor(0, 0, 0, 200);
		view->StrokeEllipse(BRect(parameters.pivot.x - parameters.pivotSize,
			parameters.pivot.y - parameters.pivotSize,
			parameters.pivot.x + parameters.pivotSize + 1,
			parameters.pivot.y + parameters.pivotSize + 1));

		view->PopState();
		view->SetFlags(flags);
	}

private:
	TransformToolState*	fState;
};


#endif // TRANSFORM_TOOL_STATE_PLATFORM_DELEGATE_H
