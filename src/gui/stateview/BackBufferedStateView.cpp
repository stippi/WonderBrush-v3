#include "BackBufferedStateView.h"

#include <new>
#include <stdio.h>

#include <Bitmap.h>
#include <Region.h>
#include <Screen.h>

using std::nothrow;

// constructor
BackBufferedStateView::BackBufferedStateView(EditContext& editContext,
		BRect frame, const char* name, uint32 resizingMode, uint32 flags)
	: StateView(editContext, frame, name, resizingMode, flags | B_FRAME_EVENTS)
	, fOffscreenBitmap(NULL)
	, fOffscreenView(NULL)
	, fSyncToRetrace(false)
{
	SetViewColor(B_TRANSPARENT_COLOR);
	_AllocBackBitmap(frame.Width(), frame.Height());
}

#ifdef __HAIKU__

// constructor
BackBufferedStateView::BackBufferedStateView(EditContext& editContext,
		const char* name, uint32 flags)
	: StateView(editContext, name, flags | B_FRAME_EVENTS)
	, fOffscreenBitmap(NULL)
	, fOffscreenView(NULL)
	, fSyncToRetrace(false)
{
	SetViewColor(B_TRANSPARENT_COLOR);
}

#endif // __HAIKU__

// destructor
BackBufferedStateView::~BackBufferedStateView()
{
	_FreeBackBitmap();
}

// #pragma mark -

// AttachedToWindow
void
BackBufferedStateView::AttachedToWindow()
{
	SyncGraphicsState();
	StateView::AttachedToWindow();
}

// FrameResized
void
BackBufferedStateView::FrameResized(float width, float height)
{
	_AllocBackBitmap(width, height);
}

// Draw
void
BackBufferedStateView::PlatformDraw(PlatformDrawContext& drawContext)
{
	BRect updateRect = drawContext.UpdateRect();

	if (!fOffscreenView) {
		DrawInto(this, updateRect);
	} else {
		BPoint boundsLeftTop = Bounds().LeftTop();
		if (fOffscreenBitmap->Lock()) {
			fOffscreenView->PushState();

			// apply scrolling offset to offscreen view
			fOffscreenView->SetOrigin(-boundsLeftTop.x, -boundsLeftTop.y);
			// mirror the clipping of this view
			// to the offscreen view for performance
			BRegion clipping;
			GetClippingRegion(&clipping);
			fOffscreenView->ConstrainClippingRegion(&clipping);

			DrawBackgroundInto(fOffscreenView, updateRect);
			DrawInto(fOffscreenView, updateRect);

			fOffscreenView->PopState();
			fOffscreenView->Sync();

			fOffscreenBitmap->Unlock();
		}
		// compensate scrolling offset in BView
		BRect bitmapRect = updateRect;
		bitmapRect.OffsetBy(-boundsLeftTop.x, -boundsLeftTop.y);

		SetDrawingMode(B_OP_COPY);
		if (fSyncToRetrace) {
			BScreen screen(Window());
			screen.WaitForRetrace();
		}
		DrawBitmap(fOffscreenBitmap, bitmapRect, updateRect);
	}
}

// #pragma mark -

// DrawBackgroundInto
void
BackBufferedStateView::DrawBackgroundInto(BView* view, BRect updateRect)
{
	view->FillRect(updateRect, B_SOLID_LOW);
}

// DrawInto
void
BackBufferedStateView::DrawInto(BView* view, BRect updateRect)
{
	PlatformDrawContext drawContext(view, updateRect);
	StateView::Draw(drawContext);
}

// SyncGraphicsState
void
BackBufferedStateView::SyncGraphicsState()
{
	if (!fOffscreenView || !fOffscreenView->LockLooper())
		return;

	BFont font;
	GetFont(&font);
	fOffscreenView->SetFont(&font);

	fOffscreenView->SetLowColor(LowColor());
	fOffscreenView->SetHighColor(HighColor());
	fOffscreenView->SetDrawingMode(DrawingMode());

	source_alpha srcAlpha;
	alpha_function alphaFunc;
	GetBlendingMode(&srcAlpha, &alphaFunc);
	fOffscreenView->SetBlendingMode(srcAlpha, alphaFunc);

	fOffscreenView->SetLineMode(LineCapMode(), LineJoinMode(),
		LineMiterLimit());

	fOffscreenView->Sync();

	fOffscreenView->UnlockLooper();
}

// SetSyncToRetrace
void
BackBufferedStateView::SetSyncToRetrace(bool sync)
{
	fSyncToRetrace = sync;
}

// #pragma mark -

// _AllocBackBitmap
void
BackBufferedStateView::_AllocBackBitmap(float width, float height)
{
	// sanity check
	if (width <= 0.0 || height <= 0.0)
		return;

	if (fOffscreenBitmap) {
		// see if the bitmap needs to be expanded
		BRect b = fOffscreenBitmap->Bounds();
		if (b.Width() >= width && b.Height() >= height)
			return;

		// it does; clean up:
		_FreeBackBitmap();
	}

	BRect b(0.0, 0.0, width, height);
	fOffscreenBitmap = new (nothrow) BBitmap(b, B_RGB32, true);
	if (!fOffscreenBitmap) {
		fprintf(stderr, "BackBufferedStateView::_AllocBackBitmap(): "
			"failed to allocate\n");
		return;
	}
	if (fOffscreenBitmap->IsValid()) {
		fOffscreenView = new BView(b, 0, B_FOLLOW_NONE, B_WILL_DRAW);
		fOffscreenBitmap->AddChild(fOffscreenView);
		SyncGraphicsState();
	} else {
		_FreeBackBitmap();
		fprintf(stderr, "BackBufferedStateView::_AllocBackBitmap(): "
			"bitmap invalid\n");
	}
}

// _FreeBackBitmap
void
BackBufferedStateView::_FreeBackBitmap()
{
	if (fOffscreenBitmap) {
		delete fOffscreenBitmap;
		fOffscreenBitmap = NULL;
		fOffscreenView = NULL;
	}
}

