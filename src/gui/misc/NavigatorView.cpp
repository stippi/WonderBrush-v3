/*
 * Copyright 2010 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "NavigatorView.h"
#include "NavigatorViewPlatformDelegate.h"

#include <new>
#include <stdio.h>

#include <Autolock.h>
#include <Bitmap.h>
#include <Cursor.h>
#include <LayoutUtils.h>
#include <Messenger.h>
#include <Region.h>

#include "ui_defines.h"

#include "Document.h"
#include "RenderManager.h"


enum {
	MSG_INVALIDATE = 'ivdt'
};

// constructor
NavigatorView::NavigatorView(Document* document, RenderManager* manager)
	: PlatformViewMixin<BView>("document icon",
		B_WILL_DRAW | B_FRAME_EVENTS | B_PULSE_NEEDED)
	, fDocument(document)
	, fRenderManager(manager)

	, fScaledBitmap(NULL)
	, fBitmapBounds(0, 0, 31, 31)
	, fDirtyDisplayArea(0, 0, -1, -1)

	, fRescaleLock("rescale bitmap lock")
	, fRescaleThread(B_BAD_THREAD_ID)
	
	, fDragStart(0, 0)
	, fDragging(false)
	, fDragMode(IGNORE_CLICK)
{
	fPlatformDelegate = new PlatformDelegate(this);
}

// destructor
NavigatorView::~NavigatorView()
{
	if (fRescaleThread >= 0) {
		int32 exitValue;
		wait_for_thread(fRescaleThread, &exitValue);
	}

	delete fScaledBitmap;

	delete fPlatformDelegate;
}

// #pragma mark -

// MessageReceived
void
NavigatorView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_BITMAP_CLEAN:
		{
			BRect area;
			if (message->FindRect("area", &area) == B_OK) {
#if NAVIGATOR_VIEW_USE_BEAUTIFUL_DOWN_SCALING
				BAutolock _(&fRescaleLock);
				if (fDirtyDisplayArea.IsValid())
					fDirtyDisplayArea = fDirtyDisplayArea | area;
				else
					fDirtyDisplayArea = area;
#else
				Invalidate(_ImageBounds());
#endif
			}
			break;
		}
		case MSG_LAYOUT_CHANGED:
		case MSG_INVALIDATE:
			Invalidate();
			break;

		default:
			BView::MessageReceived(message);
	}
}

// #pragma mark -

// Pulse
void
NavigatorView::Pulse()
{
#if NAVIGATOR_VIEW_USE_BEAUTIFUL_DOWN_SCALING
	if (!fDirtyDisplayArea.IsValid())
		return;

#if 0
	if (fScaledBitmap == NULL || fBitmapBounds != fScaledBitmap->Bounds()) {
		_AllocateBitmap(fBitmapBounds);
		Invalidate();
	}

	fDirtyDisplayArea.left = floorf(fDirtyDisplayArea.left);
	fDirtyDisplayArea.top = floorf(fDirtyDisplayArea.top);
	fDirtyDisplayArea.right = ceilf(fDirtyDisplayArea.right);
	fDirtyDisplayArea.bottom = ceilf(fDirtyDisplayArea.bottom);

	if (fRenderManager->LockDisplay()) {
		const BBitmap* bitmap = fRenderManager->DisplayBitmap();
		_RescaleBitmap(bitmap, fScaledBitmap, fDirtyDisplayArea);

		fRenderManager->UnlockDisplay();
	}

	Invalidate(_ImageBounds());

	fDirtyDisplayArea = BRect();
#else
	if (fRescaleThread >= 0)
		return;
	fRescaleThread = spawn_thread(_RescaleThreadEntry,
		"rescale display bitmap", B_LOW_PRIORITY, this);
	if (fRescaleThread >= 0)
		resume_thread(fRescaleThread);
#endif
#endif // USE_BEAUTIFUL_DOWN_SCALING
}

// AttachedToWindow
void
NavigatorView::AttachedToWindow()
{
	BMessenger* bitmapListener = new(std::nothrow) BMessenger(this);
	if (bitmapListener == NULL
		|| !fRenderManager->AddBitmapListener(bitmapListener)) {
		delete bitmapListener;
		// TODO: Bail out, throw exception or something...
	}
}

// FrameResized
void
NavigatorView::FrameResized(float width, float height)
{
	if (!fDocument->ReadLock())
		return;

	// Maintain the document aspect ratio
	float xScale = fDocument->Bounds().Width() / width;
	float yScale = fDocument->Bounds().Height() / height;
	if (xScale > yScale)
		height = floorf(fDocument->Bounds().Height() / xScale);
	else
		width = floorf(fDocument->Bounds().Width() / yScale);

	if (fabs(width - Bounds().Width()) < 2.0)
		width = Bounds().Width();
	if (fabs(height - Bounds().Height()) < 2.0)
		height = Bounds().Height();

	fDocument->ReadUnlock();

	fBitmapBounds.Set(0, 0, width, height);

	if (fRenderManager->LockDisplay()) {
		const BBitmap* bitmap = fRenderManager->DisplayBitmap();
		fDirtyDisplayArea = bitmap->Bounds();

		fRenderManager->UnlockDisplay();
	}

	Invalidate();
}


void
NavigatorView::PlatformDraw(PlatformDrawContext& drawContext)
{
	BRect bounds(Bounds());
	BRegion outside(bounds);

	// This method needs to be fast, since it will be called often.
	BRect imageBounds = _ImageBounds();
	if (drawContext.UpdateRect().Intersects(imageBounds)) {
#if NAVIGATOR_VIEW_USE_BEAUTIFUL_DOWN_SCALING
		if (fScaledBitmap != NULL) {
			BAutolock _(&fRescaleLock);
			fPlatformDelegate->DrawBitmap(drawContext, fScaledBitmap,
				imageBounds);
			outside.Exclude(imageBounds);
		}
#else
		if (fRenderManager->LockDisplay()) {
			const BBitmap* bitmap = fRenderManager->DisplayBitmap();
			fPlatformDelegate->DrawBitmap(drawContext, bitmap, imageBounds);
			fRenderManager->UnlockDisplay();
			outside.Exclude(imageBounds);
		}
#endif
	}

	fPlatformDelegate->DrawBackground(drawContext, outside);

	BRect visibleRect = _VisibleRect(imageBounds);
	if (!visibleRect.Contains(imageBounds))
		fPlatformDelegate->DrawRect(drawContext, visibleRect, imageBounds);
}


// #pragma mark -

// MouseDown
void
NavigatorView::MouseDown(BPoint where)
{
	if (fDragMode == SET_VISIBLE_RECT) {
		// Set initial rect location to clicked spot
		_SetDragMode(DRAG_VISIBLE_RECT);
		BRect visibleRect = _VisibleRect();
		fDragStartVisibleRect = fRenderManager->VisibleRect();
		BPoint center(
			(visibleRect.left + visibleRect.right) / 2.0f,
			(visibleRect.top + visibleRect.bottom) / 2.0f
		);
		_MoveVisibleRect(where - center);
	}

	if (fDragMode == DRAG_VISIBLE_RECT) {
		// begin tracking
		fDragging = true;
		fDragStart = where;
		fDragStartVisibleRect = fRenderManager->VisibleRect();

		SetMouseEventMask(B_POINTER_EVENTS,
			B_LOCK_WINDOW_FOCUS | B_SUSPEND_VIEW_FOCUS);
	}
}

// MouseUp
void
NavigatorView::MouseUp(BPoint where)
{
	fDragging = false;
}

// MouseMoved
void
NavigatorView::MouseMoved(BPoint where, uint32 transit,
	const BMessage* dragMessage)
{
	if (fDragging) {
		// Move the visible rect
		_MoveVisibleRect(where - fDragStart);
	} else {
		// Determine the drag mode for when the user clicks
		uint32 dragMode = IGNORE_CLICK;

		BRect imageBounds = _ImageBounds();
		BRect visibleBounds = _VisibleRect(imageBounds);
		if (visibleBounds.Contains(where)) {
			if (!visibleBounds.Contains(imageBounds))
				dragMode = DRAG_VISIBLE_RECT;
		} else {
			if (imageBounds.Contains(where))
				dragMode = SET_VISIBLE_RECT;
		}

		_SetDragMode(dragMode);
	}
}

// #pragma mark -

// MinSize
BSize
NavigatorView::MinSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMinSize(), BSize(31, 31));
}

// MaxSize
BSize
NavigatorView::MaxSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMaxSize(),
		BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));
}

// PreferredSize
BSize
NavigatorView::PreferredSize()
{
	return BLayoutUtils::ComposeSize(ExplicitPreferredSize(), BSize(63, 63));
}

// HasHeightForWidth
bool
NavigatorView::HasHeightForWidth()
{
	// NOTE: The height for width thing works fine, but then the view
	// can't have a smaller than optimal height and takes too much space.
//	return true;
	return false;
}

// GetHeightForWidth
void
NavigatorView::GetHeightForWidth(float width, float* min, float* max,
	float* preferred)
{
	float height = width;

	if (fDocument->ReadLock()) {
		float scale = width / fDocument->Bounds().Width();
		height = floorf(fDocument->Bounds().Height() * scale);
		fDocument->ReadUnlock();
	}

	// Prevent the view from being taller than wide. Otherwise we don't have
	// enough space for the list views
	if (height > width)
		height = width;

	if (min != NULL)
		*min = height;
	if (max != NULL)
		*max = height;
	if (preferred != NULL)
		*preferred = height;
}

// #pragma mark - private

// _ImageBounds
BRect
NavigatorView::_ImageBounds() const
{
	BRect viewBounds = Bounds();
	BRect imageBounds = fBitmapBounds;
	imageBounds.OffsetTo(
		floorf((viewBounds.Width() - imageBounds.Width()) / 2),
		floorf((viewBounds.Height() - imageBounds.Height()) / 2));
	return imageBounds;
}

// _VisibleRect
BRect
NavigatorView::_VisibleRect() const
{
	return _VisibleRect(_ImageBounds());
}

// _VisibleRect
BRect
NavigatorView::_VisibleRect(const BRect& imageBounds) const
{
	// Access to the RenderManager is save like this, since these members
	// are modified in the window thread only.
	BRect dataRect = fRenderManager->DataRect();
	BRect visibleRect = fRenderManager->VisibleRect();
	// Clip inset
	dataRect.InsetBy(-dataRect.left, -dataRect.top);
	visibleRect = dataRect & visibleRect;
	// Scale rects into imageBounds size.
	float scaleX = dataRect.Width() / imageBounds.Width();
	float scaleY = dataRect.Height() / imageBounds.Height();
	visibleRect.left /= scaleX;
	visibleRect.right /= scaleX;
	visibleRect.top /= scaleY;
	visibleRect.bottom /= scaleY;
	visibleRect.OffsetBy(imageBounds.left, imageBounds.top);
	visibleRect.OffsetBy(
		floorf(visibleRect.left + 0.5f) - visibleRect.left,
		floorf(visibleRect.top + 0.5f) - visibleRect.top);
	visibleRect.right = floorf(visibleRect.right + 0.5f);
	visibleRect.bottom = floorf(visibleRect.bottom + 0.5f);

	return visibleRect;
}

// _AllocateBitmap
void
NavigatorView::_AllocateBitmap(BRect bounds)
{
	delete fScaledBitmap;
	fScaledBitmap = new(std::nothrow) BBitmap(bounds, 0, B_RGBA32);
	if (fScaledBitmap == NULL)
		return;
	if (fScaledBitmap->InitCheck() != B_OK) {
		delete fScaledBitmap;
		fScaledBitmap = NULL;
		return;
	}

#if 1
	// TODO: This is only to confirm there is no out of bounds memory access...
	uint8* bits = static_cast<uint8*>(fScaledBitmap->Bits());
	memset(bits, 0, fScaledBitmap->BitsLength());
	uint32 bpr = fScaledBitmap->BytesPerRow();
	uint32 w = fScaledBitmap->Bounds().IntegerWidth() + 1;
	uint32 h = fScaledBitmap->Bounds().IntegerHeight() + 1;
	for (uint32 y = 0; y < h; y++) {
		uint8* p = bits;
		for (uint32 x = 0; x < w; x++) {
			p[2] = 255;
			p += 4;
		}
		bits += bpr;
	}
#endif
}

// _RescaleBitmap
void
NavigatorView::_RescaleBitmap(const BBitmap* source, const BBitmap* dest,
	BRect sourceArea)
{
	if (source == NULL || dest == NULL || !sourceArea.IsValid()
		|| !sourceArea.Intersects(source->Bounds())) {
		return;
	}

	uint8* src = static_cast<uint8*>(source->Bits());
	uint32 srcBPR = source->BytesPerRow();
	uint32 srcWidth = source->Bounds().IntegerWidth() + 1;
	uint32 srcHeight = source->Bounds().IntegerHeight() + 1;

	uint8* dst = static_cast<uint8*>(dest->Bits());
	uint32 dstBPR = dest->BytesPerRow();
	uint32 dstWidth = dest->Bounds().IntegerWidth() + 1;
	uint32 dstHeight = dest->Bounds().IntegerHeight() + 1;

	float xScale = (float)(srcWidth - 1) / dstWidth;
	float yScale = (float)(srcHeight - 1) / dstHeight;

	// TODO: Limit to sourceArea!

	// Intermediate buffer has a width of the target bitmap but a height of
	// the source bitmap. The conversion will be two pass. In the first pass,
	// the source is expanded to 16 bit linear RGB and scaled horizontally,
	// in the second pass it is scaled vertically and converted back to sRGB.
	uint32 intermediateBPR = dstWidth * 4;
	uint16* intermediateBuffer
		= new(std::nothrow) uint16[srcHeight * intermediateBPR];
	uint16* intermediateRow = intermediateBuffer;

//bigtime_t now = system_time();
	// First pass: expand and scale horizontally
	uint32 xScaleInt = (uint32)(xScale * 256);
	for (uint32 y = 0; y < srcHeight; y++) {
		uint8* s = src;
		uint16* d = intermediateRow;

		uint64 sumR = 0;
		uint64 sumG = 0;
		uint64 sumB = 0;
		uint64 sumA = 0;

		uint64 maxSum = 0;

		uint32 dstStopX = 1;
		uint32 stop = dstStopX * xScaleInt;
		uint32 stopX = stop >> 8;
		stop = stop & 0xff;

		for (uint32 x = 0; x < srcWidth; x++) {
			// TODO: Use real conversion to linear RGB!
			uint16 r = (s[0] << 8) | s[0];
			uint16 g = (s[1] << 8) | s[1];
			uint16 b = (s[2] << 8) | s[2];
			uint16 a = (s[3] << 8) | s[3];
			uint16 max = 65535;

			if (x < stopX) {
				// Pixel fully within chunk
				sumR += r;
				sumG += g;
				sumB += b;
				sumA += a;
				maxSum += max;
			} else {
				// Last pixel of chunk, add partially only
				uint32 r1 = (r * stop) >> 8;
				uint32 g1 = (g * stop) >> 8;
				uint32 b1 = (b * stop) >> 8;
				uint32 a1 = (a * stop) >> 8;
				uint32 max1 = (max * stop) >> 8;
				sumR += r1;
				sumG += g1;
				sumB += b1;
				sumA += a1;
				maxSum += max1;

				// Compute average...
				d[0] = (uint16)(sumR * 65535 / maxSum);
				d[1] = (uint16)(sumG * 65535 / maxSum);
				d[2] = (uint16)(sumB * 65535 / maxSum);
				d[3] = (uint16)(sumA * 65535 / maxSum);
				// ...and go to next pixel
				d += 4;
				dstStopX++;

				// Carry rest to next pixel
				sumR = r - r1;
				sumG = g - g1;
				sumB = b - b1;
				sumA = a - a1;
				maxSum = max - max1;

				// Compute next stop
				stop = dstStopX * xScaleInt;
				stopX = stop >> 8;
				stop = stop & 0xff;
			}

			s += 4;
		}

		src += srcBPR;
		intermediateRow += intermediateBPR;
	}

	intermediateRow = intermediateBuffer;

	// Second pass: Scale intermediate buffer vertically and convert back to
	// sRGB
	uint32 yScaleInt = (uint32)(yScale * 256);
	for (uint32 x = 0; x < dstWidth; x++) {
		uint16* s = intermediateRow;
		uint8* d = dst;

		uint64 sumR = 0;
		uint64 sumG = 0;
		uint64 sumB = 0;
		uint64 sumA = 0;

		uint64 maxSum = 0;

		uint32 dstStopY = 1;
		uint32 stop = dstStopY * yScaleInt;
		uint32 stopY = stop >> 8;
		stop = stop & 0xff;

		for (uint32 y = 0; y < srcHeight; y++) {
			uint16 r = s[0];
			uint16 g = s[1];
			uint16 b = s[2];
			uint16 a = s[3];
			uint16 max = 65535;

			if (y < stopY) {
				// Pixel fully within chunk
				sumR += r;
				sumG += g;
				sumB += b;
				sumA += a;
				maxSum += max;
			} else {
				// Last pixel of chunk, add partially only
				uint32 r1 = (r * stop) >> 8;
				uint32 g1 = (g * stop) >> 8;
				uint32 b1 = (b * stop) >> 8;
				uint32 a1 = (a * stop) >> 8;
				uint32 max1 = (max * stop) >> 8;
				sumR += r1;
				sumG += g1;
				sumB += b1;
				sumA += a1;
				maxSum += max1;

				// Compute average (and convert back to 8 bits/pixel in one
				// step)...
				d[0] = (uint8)(sumR * 255 / maxSum);
				d[1] = (uint8)(sumG * 255 / maxSum);
				d[2] = (uint8)(sumB * 255 / maxSum);
				d[3] = (uint8)(sumA * 255 / maxSum);
				// ...and go to next pixel
				d += dstBPR;
				dstStopY++;

				// Carry rest to next pixel
				sumR = r - r1;
				sumG = g - g1;
				sumB = b - b1;
				sumA = a - a1;
				maxSum = max - max1;

				// Compute next stop
				stop = dstStopY * yScaleInt;
				stopY = stop >> 8;
				stop = stop & 0xff;
			}

			s += intermediateBPR;
		}

		intermediateRow += 4;
		dst += 4;
	}
//printf("down scaling: %lldµs\n", system_time() - now);

	delete[] intermediateBuffer;
}

// 
/*static*/ int32
NavigatorView::_RescaleThreadEntry(void* cookie)
{
	NavigatorView* self = static_cast<NavigatorView*>(cookie);
	return self->_RescaleThread();
}

// _RescaleThread
int32
NavigatorView::_RescaleThread()
{
	BAutolock _(&fRescaleLock);

	if (fScaledBitmap == NULL || fBitmapBounds != fScaledBitmap->Bounds())
		_AllocateBitmap(fBitmapBounds);

	fDirtyDisplayArea.left = floorf(fDirtyDisplayArea.left);
	fDirtyDisplayArea.top = floorf(fDirtyDisplayArea.top);
	fDirtyDisplayArea.right = ceilf(fDirtyDisplayArea.right);
	fDirtyDisplayArea.bottom = ceilf(fDirtyDisplayArea.bottom);

	if (fRenderManager->LockDisplay()) {
		const BBitmap* bitmap = fRenderManager->DisplayBitmap();
		_RescaleBitmap(bitmap, fScaledBitmap, fDirtyDisplayArea);

		fRenderManager->UnlockDisplay();
	}

	BMessenger messenger(this);
	if (messenger.IsValid()) {
		BMessage message(MSG_INVALIDATE);
		messenger.SendMessage(&message);
	}

	fRescaleThread = B_BAD_THREAD_ID;
	return 0;
}

// _SetDragMode
void
NavigatorView::_SetDragMode(uint32 mode)
{
	if (fDragMode == mode)
		return;
	
	fDragMode = mode;

	// Adopt view cursor according to drag mode
	BCursorID cursorID;

	switch (mode) {
		case DRAG_VISIBLE_RECT:
			cursorID = B_CURSOR_ID_MOVE;
			break;
		case SET_VISIBLE_RECT:
			cursorID = B_CURSOR_ID_FOLLOW_LINK;
			break;
		case IGNORE_CLICK:
		default:
			cursorID = B_CURSOR_ID_SYSTEM_DEFAULT;
			break;
	}

	BCursor cursor(cursorID);
	SetViewCursor(&cursor);
}

// _MoveVisibleRect
void
NavigatorView::_MoveVisibleRect(BPoint offset)
{
	if (offset == B_ORIGIN)
		return;

	BRect dataRect(fRenderManager->DataRect());
	BRect visibleRect(fDragStartVisibleRect);

	// Scale offset into dataRect size
//	dataRect.InsetBy(-dataRect.left, -dataRect.top);
	BRect imageBounds(_ImageBounds());
	float scaleX = dataRect.Width() / imageBounds.Width();
	float scaleY = dataRect.Height() / imageBounds.Height();
	offset.x *= scaleX;
	offset.y *= scaleY;

	float maxOffsetX = dataRect.right - visibleRect.right;
	float minOffsetX = dataRect.left - visibleRect.left;
	float maxOffsetY = dataRect.bottom - visibleRect.bottom;
	float minOffsetY = dataRect.top - visibleRect.top;
	
	if (offset.x < minOffsetX)
		offset.x = minOffsetX;
	if (offset.x > maxOffsetX)
		offset.x = maxOffsetX;
	if (offset.y < minOffsetY)
		offset.y = minOffsetY;
	if (offset.y > maxOffsetY)
		offset.y = maxOffsetY;

	if (offset == B_ORIGIN)
		return;

	visibleRect.OffsetBy(offset);
	fRenderManager->SetCanvasLayout(dataRect, visibleRect);
}

