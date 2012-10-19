#include <Bitmap.h>


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
