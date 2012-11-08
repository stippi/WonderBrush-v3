#include <Bitmap.h>

#include <new>


BBitmap::BBitmap(BRect bounds, uint32 flags, color_space colorSpace,
	int32 bytesPerRow, screen_id screenID)
	:
	fData(NULL),
	fOwnsData(false),
	fSize(0),
	fBytesPerRow(0),
	fColorSpace(B_NO_COLOR_SPACE),
	fImage(NULL)
{
	_Init(bounds, flags, colorSpace, bytesPerRow, screenID);
}


BBitmap::BBitmap(BRect bounds, color_space colorSpace, bool acceptsViews,
	bool needsContiguous)
	:
	fData(NULL),
	fOwnsData(false),
	fSize(0),
	fBytesPerRow(0),
	fColorSpace(B_NO_COLOR_SPACE),
	fImage(NULL)
{
	uint32 flags = (acceptsViews ? B_BITMAP_ACCEPTS_VIEWS : 0)
		| (needsContiguous ? B_BITMAP_IS_CONTIGUOUS : 0);
	_Init(bounds, flags, colorSpace, B_ANY_BYTES_PER_ROW, B_MAIN_SCREEN_ID);
}


BBitmap::BBitmap(const QImage& _image)
	:
	fData(NULL),
	fOwnsData(false),
	fSize(0),
	fBytesPerRow(0),
	fColorSpace(B_NO_COLOR_SPACE),
	fImage(NULL)
{
	QImage image = _image;

	switch (image.format()) {
		case QImage::Format_RGB32:
			fColorSpace = B_RGB32;
			break;
		case QImage::Format_ARGB32:
			fColorSpace = B_RGBA32;
			break;
		default:
		{
			QImage::Format format;
			if (image.hasAlphaChannel()) {
				format = QImage::Format_ARGB32;
				fColorSpace = B_RGBA32;
			} else {
				format = QImage::Format_RGB32;
				fColorSpace = B_RGB32;
			}
			image =	image.convertToFormat(format);
			break;
		}
	}

	fImage = new(std::nothrow) QImage(image);
	if (fImage == NULL || fImage->isNull()) {
		delete fImage;
		fImage  = NULL;
		fColorSpace = B_NO_COLOR_SPACE;
	} else {
		fData = (uint8*)fImage->bits();
		fSize = fImage->byteCount();
		fBytesPerRow = fImage->bytesPerLine();
	}
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

	if (fOwnsData)
		free(fData);
	fData = NULL;
	fOwnsData = false;

	fSize = 0;
	fBytesPerRow = 0;
	fColorSpace = B_NO_COLOR_SPACE;
}


status_t
BBitmap::InitCheck() const
{
	return IsValid() ? B_OK : B_ERROR;
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


QIcon
BBitmap::ToQIcon() const
{
	if (fImage == NULL)
		return QIcon();

	QPixmap pixmap;
	pixmap.convertFromImage(*fImage);
	return QIcon(pixmap);
}


status_t
BBitmap::_Init(BRect bounds, uint32 flags, color_space colorSpace,
	int32 bytesPerRow, screen_id screenID)
{
	// TODO: Handle flags!

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
			return B_BAD_VALUE;
	}
	fColorSpace = colorSpace;

	// check size
	int width = bounds.IntegerWidth() + 1;
	int height = bounds.IntegerHeight() + 1;
	if (width <= 0 || height <= 0)
		return B_BAD_VALUE;

	// allocate bitmap data
	fBytesPerRow = bytesPerRow == B_ANY_BYTES_PER_ROW
		? width * formatBytesPerPixel : bytesPerRow;
	fSize = height * fBytesPerRow;
	fData = (uint8*)malloc(fSize);
	if (fData == NULL) {
		Unset();
		return B_NO_MEMORY;
	}
	fOwnsData = true;

	// allocate QImage
	fImage = new(std::nothrow) QImage((uchar*)fData, width, height, fBytesPerRow,
		imageFormat);
	if (fImage == NULL || fImage->isNull()) {
		Unset();
		return B_NO_MEMORY;
	}

	return B_OK;
}
