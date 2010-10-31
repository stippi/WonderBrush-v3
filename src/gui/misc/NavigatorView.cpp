/*
 * Copyright 2010 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "NavigatorView.h"

#include <new>
#include <stdio.h>

#include <Autolock.h>
#include <Bitmap.h>
#include <LayoutUtils.h>
#include <Messenger.h>
#include <Region.h>

#include "ui_defines.h"

#include "Document.h"
#include "RenderManager.h"

#define USE_BEAUTIFUL_DOWN_SCALING 0

enum {
	MSG_INVALIDATE = 'ivdt'
};

// constructor
NavigatorView::NavigatorView(Document* document, RenderManager* manager)
	: BView("document icon", B_WILL_DRAW | B_FRAME_EVENTS | B_PULSE_NEEDED)
	, fDocument(document)
	, fRenderManager(manager)

	, fScaledBitmap(NULL)
	, fBitmapBounds(0, 0, 31, 31)
	, fDirtyDisplayArea(0, 0, -1, -1)

	, fRescaleLock("rescale bitmap lock")
	, fRescaleThread(B_BAD_THREAD_ID)
{
	SetViewColor(B_TRANSPARENT_COLOR);
	SetHighColor(kStripesHigh);
	SetLowColor(kStripesLow);
		// used for drawing the stripes pattern
}

// destructor
NavigatorView::~NavigatorView()
{
	if (fRescaleThread >= 0) {
		int32 exitValue;
		wait_for_thread(fRescaleThread, &exitValue);
	}

	delete fScaledBitmap;
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
#if USE_BEAUTIFUL_DOWN_SCALING
				BAutolock _(&fRescaleLock);
				if (fDirtyDisplayArea.IsValid())
					fDirtyDisplayArea = fDirtyDisplayArea | area;
				else
					fDirtyDisplayArea = area;
#else
				Invalidate(_IconBounds());
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
#if USE_BEAUTIFUL_DOWN_SCALING
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

	Invalidate(_IconBounds());

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

// Draw
void
NavigatorView::Draw(BRect updateRect)
{
	BRect bounds(Bounds());
	BRegion outside(bounds);

	// This method needs to be fast, since it will be called often.
	BRect iconBounds = _IconBounds();
	if (updateRect.Intersects(iconBounds)) {
#if USE_BEAUTIFUL_DOWN_SCALING
		if (fScaledBitmap != NULL) {
			BAutolock _(&fRescaleLock);
			DrawBitmapAsync(fScaledBitmap, fScaledBitmap->Bounds(),
				iconBounds, B_FILTER_BITMAP_BILINEAR);
			outside.Exclude(iconBounds);
		}
#else
		if (fRenderManager->LockDisplay()) {
			const BBitmap* bitmap = fRenderManager->DisplayBitmap();
			DrawBitmapAsync(bitmap, bitmap->Bounds(),
				iconBounds, B_FILTER_BITMAP_BILINEAR);
			outside.Exclude(iconBounds);
			fRenderManager->UnlockDisplay();
		}
#endif
	}

	FillRegion(&outside, kStripes);

	// Access to the RenderManager is save like this, since these members
	// are modified in the window thread only.
	BRect dataRect = fRenderManager->DataRect();
	BRect visibleRect = fRenderManager->VisibleRect();
	// Clip inset
	dataRect.InsetBy(-dataRect.left, -dataRect.top);
	visibleRect = dataRect & visibleRect;
	// Scale rects into iconBounds size.
	float scaleX = dataRect.Width() / iconBounds.Width();
	float scaleY = dataRect.Height() / iconBounds.Height();
	visibleRect.left /= scaleX;
	visibleRect.right /= scaleX;
	visibleRect.top /= scaleY;
	visibleRect.bottom /= scaleY;
	visibleRect.OffsetBy(iconBounds.left, iconBounds.top);
	visibleRect.OffsetBy(
		floorf(visibleRect.left + 0.5f) - visibleRect.left,
		floorf(visibleRect.top + 0.5f) - visibleRect.top);
	visibleRect.right = floorf(visibleRect.right + 0.5f);
	visibleRect.bottom = floorf(visibleRect.bottom + 0.5f);
	if (!visibleRect.Contains(iconBounds)) {
		SetHighColor(255, 255, 255, 170);
		SetLowColor(0, 0, 0);
		SetDrawingMode(B_OP_ALPHA);
		SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_OVERLAY);
		StrokeRect(visibleRect, kDotted);
		visibleRect.InsetBy(1, 1);
		StrokeRect(visibleRect, kDotted);

		outside.Set(iconBounds);
		outside.Exclude(visibleRect);
		SetHighColor(0, 0, 0, 50);
		FillRegion(&outside);
	}
}

// #pragma mark -

// MouseDown
void
NavigatorView::MouseDown(BPoint where)
{
	// TODO: begin tracking
}

// MouseUp
void
NavigatorView::MouseUp(BPoint where)
{
	// TODO: stop tracking
}

// MouseMoved
void
NavigatorView::MouseMoved(BPoint where, uint32 transit,
	const BMessage* dragMessage)
{
	// TODO: if tracked distance threshold, start dragging the document
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

// _IconBounds
BRect
NavigatorView::_IconBounds() const
{
	BRect viewBounds = Bounds();
	BRect iconBounds = fBitmapBounds;
	iconBounds.OffsetTo(
		floorf((viewBounds.Width() - iconBounds.Width()) / 2),
		floorf((viewBounds.Height() - iconBounds.Height()) / 2));
	return iconBounds;
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

