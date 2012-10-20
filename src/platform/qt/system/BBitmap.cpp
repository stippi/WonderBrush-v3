#include <Bitmap.h>


BBitmap::BBitmap(BRect bounds, uint32 flags, color_space colorSpace,
	int32 bytesPerRow, screen_id screenID)
{
	// TODO:...
}


BBitmap::~BBitmap()
{
}


BArchivable*
BBitmap::Instantiate(BMessage* data)
{
// TODO:...
	return NULL;
}


status_t
BBitmap::Archive(BMessage* data, bool deep) const
{
// TODO:...
	return B_BAD_VALUE;
}


void*
BBitmap::Bits() const
{
// TODO:...
	return NULL;
}


int32
BBitmap::BitsLength() const
{
	return fSize;
}


int32
BBitmap::BytesPerRow() const
{
	return fBytesPerRow;
}


color_space
BBitmap::ColorSpace() const
{
	return fColorSpace;
}


BRect
BBitmap::Bounds() const
{
	return fBounds;
}


bool
BBitmap::IsValid() const
{
// TODO:...
	return false;
}
