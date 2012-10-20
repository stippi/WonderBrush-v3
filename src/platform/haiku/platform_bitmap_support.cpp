#include "bitmap_support.h"

#include <new>

#include <Bitmap.h>
#include <View.h>


BBitmap*
scale_bitmap(const BBitmap* bitmap, BRect newBounds)
{
	// create the new bitmap
	BBitmap* scaledBitmap = new(std::nothrow) BBitmap(newBounds,
		B_BITMAP_ACCEPTS_VIEWS, B_RGBA32);
	if (scaledBitmap == NULL || !scaledBitmap->IsValid()) {
		delete scaledBitmap;
		return NULL;
	}

	// scale by drawing the old bitmap into the new one
	BView view(newBounds, "temp", 0, 0);
	scaledBitmap->Lock();
	scaledBitmap->AddChild(&view);
	view.DrawBitmap(bitmap, bitmap->Bounds(), newBounds,
		B_FILTER_BITMAP_BILINEAR);
	view.Sync();
	view.RemoveSelf();
	scaledBitmap->Unlock();

	return scaledBitmap;
}
