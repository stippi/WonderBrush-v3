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

	void DrawControlPoint(BView* view, const BPoint& point,
		bool selected, bool highlight,
		const rgb_color& highlightColor, const rgb_color& focusColor)
	{
		const float kControlPointExtent = 2.0f;

		if (highlight)
			view->SetLowColor(highlightColor);
		else if (selected)
			view->SetLowColor(focusColor);
		else
			view->SetLowColor(kBlack);

		if (selected)
			view->SetHighColor(220, 220, 220, 255);
		else
			view->SetHighColor(170, 170, 170, 255);

		BRect r(
			point.x - kControlPointExtent,
			point.y - kControlPointExtent,
			point.x + kControlPointExtent,
			point.y + kControlPointExtent
		);
		view->StrokeEllipse(r, B_SOLID_LOW);

		r.InsetBy(1.0, 1.0);
		view->FillEllipse(r, B_SOLID_HIGH);
	}

	void DrawPathPoint(BView* view, const BPoint& point,
		bool selected, bool highlight,
		const rgb_color& highlightColor, const rgb_color& focusColor)
	{
		const float kPointExtent = 3.0f;

		if (highlight)
			view->SetLowColor(highlightColor);
		else if (selected)
			view->SetLowColor(focusColor);
		else
			view->SetLowColor(kBlack);

		view->SetDrawingMode(B_OP_COPY);
		BRect r(point, point);
		r.InsetBy(-kPointExtent, -kPointExtent);
		view->StrokeRect(r, B_SOLID_LOW);
		r.InsetBy(1.0, 1.0);
		view->FillRect(r, B_SOLID_HIGH);
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
				fView->SetHighColor(52, 52, 52, 255);
				fView->SetLowColor(203, 203, 203, 255);
				fView->SetDrawingMode(B_OP_COPY);
			}

			virtual ~StrokePathIterator()
			{
			}

			virtual void MoveTo(BPoint point)
			{
				fBlack = true;
				fSkip = false;

				fViewState.TransformObjectToView(&point, false);
				fView->MovePenTo(point);
			}

			virtual void LineTo(BPoint point)
			{
				fViewState.TransformObjectToView(&point, false);
				if (!fSkip) {
					if (fBlack)
						fView->StrokeLine(point, B_SOLID_HIGH);
					else
						fView->StrokeLine(point, B_SOLID_LOW);
					fBlack = !fBlack;

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
			if (point != pointIn)
				view->StrokeLine(point, pointIn);
			if (point != pointOut)
				view->StrokeLine(point, pointOut);

			// draw main control point
			DrawPathPoint(view, point,
				selection.Contains(PathPoint(path, i, POINT)),
				highlight && (hoverPoint.GetWhich() & POINT) != 0,
				highlightColor, focusColor);

			// draw in control point
			if (pointIn != point) {
				DrawControlPoint(view, pointIn,
					selection.Contains(PathPoint(path, i, POINT_IN)),
					highlight && (hoverPoint.GetWhich() & POINT_IN) != 0,
					highlightColor, focusColor);
			}

			// draw out control point
			if (pointOut != point) {
				DrawControlPoint(view, pointOut,
					selection.Contains(PathPoint(path, i, POINT_OUT)),
					highlight && (hoverPoint.GetWhich() & POINT_OUT) != 0,
					highlightColor, focusColor);
			}
		}
	}

private:
	PathToolState*	fState;
};


#endif // PATH_TOOL_STATE_PLATFORM_DELEGATE_H
