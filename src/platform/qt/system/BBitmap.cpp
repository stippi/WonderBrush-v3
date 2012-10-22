#include <Bitmap.h>

#include <new>


BBitmap::BBitmap(BRect bounds, uint32 flags, color_space colorSpace,
	int32 bytesPerRow, screen_id screenID)
	:
	fData(NULL),
	fSize(0),
	fBytesPerRow(0),
	fColorSpace(B_NO_COLOR_SPACE),
	fImage(NULL)
{
	// Check color space. We don't support a lot of formats ATM.
	QImage::Format imageFormat;
	int32 formatBytesPerPixel;
	switch (colorSpace) {
		case B_RGB32:
			imageFormat = QImage::Format_RGB32;
			formatBytesPerPixel = 4;
			break;
		case B_RGBA32:
			imageFormat = QImage::Format_ARGB32;
			formatBytesPerPixel = 4;
			break;
		default:
			return;
	}
	fColorSpace = colorSpace;

	// check size
	int width = bounds.IntegerWidth() + 1;
	int height = bounds.IntegerHeight() + 1;
	if (width <= 0 || height <= 0)
		return;

	// allocate bitmap data
	fBytesPerRow = bytesPerRow == B_ANY_BYTES_PER_ROW
		? width * formatBytesPerPixel : bytesPerRow;
	fSize = height * fBytesPerRow;
	fData = (uint8*)malloc(fSize);
	if (fData == NULL) {
		Unset();
		return;
	}

	// allocate QImage
	fImage = new(std::nothrow) QImage((uchar*)fData, width, height, fBytesPerRow,
		imageFormat);
	if (fImage == NULL || fImage->isNull()) {
		Unset();
		return;
	}
// TODO: Handle flags!
}


BBitmap::~BBitmap()
{
	Unset();
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


void
BBitmap::Unset()
{
	delete fImage;
	fImage = NULL;
	free(fData);
	fData = NULL;
	fSize = 0;
	fBytesPerRow = 0;
	fColorSpace = B_NO_COLOR_SPACE;
}


bool
BBitmap::IsValid() const
{
	return fImage != NULL;
}


void*
BBitmap::Bits() const
{
	return fData;
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
	return fImage != NULL
		? BRect(0, 0, fImage->width() - 1, fImage->height() - 1)
		: BRect();
}
