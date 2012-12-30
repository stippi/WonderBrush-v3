#ifndef PATH_TOOL_STATE_PLATFORM_DELEGATE_H
#define PATH_TOOL_STATE_PLATFORM_DELEGATE_H


#include <Shape.h>

#include "PathToolState.h"
#include "ui_defines.h"


class PathToolState::PlatformDelegate {
public:
	PlatformDelegate(PathToolState* state)
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

	void DrawPath(Path* path, PlatformDrawContext& drawContext,
		TransformViewState& viewState, const PointSelection& selection,
		const PathPoint& hoverPoint, float zoomLevel)
	{
		class StrokePathIterator : public Path::Iterator {
		public:
			StrokePathIterator(TransformViewState& viewState, BView* view)
				: fViewState(viewState)
				, fView(view)
			{
				fView->SetHighColor(0, 0, 0, 255);
				fView->SetDrawingMode(B_OP_OVER);
			}

			virtual ~StrokePathIterator()
			{
			}

			virtual void MoveTo(BPoint point)
			{
				fBlack = true;
				fSkip = false;
				fView->SetHighColor(0, 0, 0, 255);

				fViewState.TransformObjectToView(&point, false);
				fView->MovePenTo(point);
			}

			virtual void LineTo(BPoint point)
			{
				fViewState.TransformObjectToView(&point, false);
				if (!fSkip) {
					if (fBlack)
						fView->SetHighColor(255, 255, 255, 255);
					else
						fView->SetHighColor(0, 0, 0, 255);
					fBlack = !fBlack;

					fView->StrokeLine(point);
				} else {
					fView->MovePenTo(point);
				}
				fSkip = !fSkip;
			}

		private:
			TransformViewState&		fViewState;
			BView*					fView;
			bool					fBlack;
			bool					fSkip;
		};

		BView* view = drawContext.View();

		uint32 flags = view->Flags();
		view->SetFlags(flags | B_SUBPIXEL_PRECISE);

		StrokePathIterator iterator(viewState, view);
		path->Iterate(&iterator, zoomLevel);

		view->SetFlags(flags);

		const float kPointExtent = 3.0f;
		const float kControlPointExtent = 1.0f;

		view->SetLowColor(0, 0, 0, 255);
		BPoint point;
		BPoint pointIn;
		BPoint pointOut;
		rgb_color focusColor = (rgb_color){ 255, 0, 0, 255 };
		rgb_color highlightColor = (rgb_color){ 60, 60, 255, 255 };
		for (int32 i = 0; path->GetPointsAt(i, point, pointIn, pointOut); i++) {
			PathPoint pathPoint(path, i, POINT_ALL);
			bool highlight = hoverPoint.IsSameIndex(pathPoint);
			view->SetHighColor(255, 255, 255, 255);
			// convert to view coordinate space
			viewState.TransformObjectToView(&point, true);
			viewState.TransformObjectToView(&pointIn, true);
			viewState.TransformObjectToView(&pointOut, true);
			// connect the points belonging to one control point
			view->SetDrawingMode(B_OP_INVERT);
			view->StrokeLine(point, pointIn);
			view->StrokeLine(point, pointOut);
			// draw main control point
			if (highlight && (hoverPoint.GetWhich() & POINT) != 0)
				view->SetLowColor(highlightColor);
			if (selection.Contains(PathPoint(path, i, POINT)))
				view->SetLowColor(focusColor);
			else
				view->SetLowColor(kBlack);

			view->SetDrawingMode(B_OP_COPY);
			BRect r(point, point);
			r.InsetBy(-kPointExtent, -kPointExtent);
			view->StrokeRect(r, B_SOLID_LOW);
			r.InsetBy(1.0, 1.0);
			view->FillRect(r, B_SOLID_HIGH);
			// draw in control point
			if (highlight && (hoverPoint.GetWhich() & POINT_IN) != 0)
				view->SetLowColor(highlightColor);
			else if (selection.Contains(PathPoint(path, i, POINT_IN)))
				view->SetLowColor(focusColor);
			else
				view->SetLowColor(kBlack);
			if (selection.Contains(PathPoint(path, i, POINT_IN)))
				view->SetHighColor(220, 220, 220, 255);
			else
				view->SetHighColor(170, 170, 170, 255);
			if (pointIn != point) {
				r.Set(
					pointIn.x - kControlPointExtent,
					pointIn.y - kControlPointExtent,
					pointIn.x + kControlPointExtent,
					pointIn.y + kControlPointExtent
				);
				view->StrokeRect(r, B_SOLID_LOW);
				r.InsetBy(1.0, 1.0);
				view->FillRect(r, B_SOLID_HIGH);
			}
			// draw out control point
			if (highlight && (hoverPoint.GetWhich() & POINT_OUT) != 0)
				view->SetLowColor(highlightColor);
			else if (selection.Contains(PathPoint(path, i, POINT_OUT)))
				view->SetLowColor(focusColor);
			else
				view->SetLowColor(kBlack);
			if (selection.Contains(PathPoint(path, i, POINT_OUT)))
				view->SetHighColor(220, 220, 220, 255);
			else
				view->SetHighColor(170, 170, 170, 255);
			if (pointOut != point) {
				r.Set(
					pointOut.x - kControlPointExtent,
					pointOut.y - kControlPointExtent,
					pointOut.x + kControlPointExtent,
					pointOut.y + kControlPointExtent
				);
				view->StrokeRect(r, B_SOLID_LOW);
				r.InsetBy(1.0, 1.0);
				view->FillRect(r, B_SOLID_HIGH);
			}
		}
	}

	void DrawInvertedShape(PlatformDrawContext& drawContext, BShape& shape)
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
	PathToolState*	fState;
};


#endif // PATH_TOOL_STATE_PLATFORM_DELEGATE_H
