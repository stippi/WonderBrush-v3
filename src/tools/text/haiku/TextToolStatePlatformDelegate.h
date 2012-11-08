#ifndef TEXT_TOOL_STATE_PLATFORM_DELEGATE_H
#define TEXT_TOOL_STATE_PLATFORM_DELEGATE_H


#include <Shape.h>

#include "TextToolState.h"


class TextToolState::PlatformDelegate {
public:
	typedef BShape Polygon;

	struct PolygonBuilder {
		PolygonBuilder()
			:
			fInitialPoint(true)
		{
		}

		PolygonBuilder& operator<<(const BPoint& point)
		{
			if (fInitialPoint) {
				fPolygon.MoveTo(point);
				fInitialPoint = false;
			} else
				fPolygon.LineTo(point);

			return *this;
		}

		Polygon& GetPolygon()
		{
			fPolygon.Close();
			return fPolygon;
		}

	private:
		Polygon	fPolygon;
		bool	fInitialPoint;
	};

public:
	PlatformDelegate(TextToolState* state)
		:
		fState(state)
	{
	}

	void DrawControls(PlatformDrawContext& drawContext, const BPoint& origin,
		const BPoint& widthOffset)
	{
		BView* view = drawContext.View();

		view->SetHighColor(0, 0, 0, 200);
		view->SetPenSize(3.0);
		view->StrokeLine(origin, widthOffset);
		view->SetHighColor(255, 255, 255, 200);
		view->SetPenSize(1.0);
		view->StrokeLine(origin, widthOffset);

		float size = 3;
		view->SetHighColor(255, 255, 255, 200);
		view->FillEllipse(BRect(origin.x - size, origin.y - size,
			origin.x + size + 0.5, origin.y + size + 0.5));
		view->SetHighColor(0, 0, 0, 200);
		view->StrokeEllipse(BRect(origin.x - size, origin.y - size,
			origin.x + size + 1, origin.y + size + 1));

		size = 2;
		view->SetHighColor(255, 255, 255, 200);
		view->FillRect(BRect(widthOffset.x - size, widthOffset.y - size,
			widthOffset.x + size + 0.5, widthOffset.y + size + 0.5));
		view->SetHighColor(0, 0, 0, 200);
		view->StrokeRect(BRect(widthOffset.x - size, widthOffset.y - size,
			widthOffset.x + size + 1, widthOffset.y + size + 1));
	}

	void DrawInvertedPolygon(PlatformDrawContext& drawContext,
		Polygon& shape)
	{
		BView* view = drawContext.View();
		view->PushState();
		uint32 flags = view->Flags();
		view->SetFlags(flags | B_SUBPIXEL_PRECISE);
		view->SetDrawingMode(B_OP_INVERT);
		view->MovePenTo(B_ORIGIN);
		view->FillShape(&shape);
		view->SetFlags(flags);
		view->PopState();
	}

private:
	TextToolState*	fState;
};


#endif // TEXT_TOOL_STATE_PLATFORM_DELEGATE_H
