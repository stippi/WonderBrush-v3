#ifndef PATH_TOOL_STATE_PLATFORM_DELEGATE_H
#define PATH_TOOL_STATE_PLATFORM_DELEGATE_H


#include <Shape.h>

#include "PathToolState.h"


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

	void DrawPath(const Path& path, PlatformDrawContext& drawContext,
		TransformViewState& viewState, float zoomLevel)
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
		path.Iterate(&iterator, zoomLevel);

		view->SetFlags(flags);
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
