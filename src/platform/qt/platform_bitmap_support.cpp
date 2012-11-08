#include "bitmap_support.h"

#include <new>

#include <Bitmap.h>


BBitmap*
scale_bitmap(const BBitmap* bitmap, BRect newBounds)
{
	if (bitmap == NULL || bitmap->GetQImage() == NULL)
		return NULL;

	QImage scaledImage = bitmap->GetQImage()->scaled(
		newBounds.IntegerWidth() + 1, newBounds.IntegerHeight() + 1,
		Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	BBitmap* scaledBitmap = new(std::nothrow) BBitmap(scaledImage);
	if (scaledBitmap == NULL || !scaledBitmap->IsValid()) {
		delete scaledBitmap;
		return NULL;
	}
	return scaledBitmap;
}

