#include "IconButton.h"

#include <stdio.h>

#include <new>

#include <Bitmap.h>
#include <Control.h>
#include <IconUtils.h>
#include <Resources.h>

#include "support.h"


//IconButton::IconButton(QWidget* parent)
//	:
//	QToolButton(parent)
//{
//}


IconButton::IconButton(const char* name, uint32 id, const char* label,
	BMessage* message, BHandler* target)
	:
	QToolButton(),
	BHandler(name),
	BInvoker(message, target),
	fID(id),
	fNormalBitmap(NULL),
	fNormalIcon(NULL),
	fDisabledIcon(NULL),
	fClickedIcon(NULL),
	fDisabledClickedIcon(NULL)
{
	setAutoRaise(true);

	SetRadioMode(false);

	connect(this, SIGNAL(pressed()), SLOT(_UpdateIcon()));
	connect(this, SIGNAL(released()), SLOT(_UpdateIcon()));
}


IconButton::~IconButton()
{
	_DeleteBitmaps();
}


status_t
IconButton::Invoke(BMessage* message)
{
	if (!message)
		message = Message();
	if (message) {
		BMessage clone(*message);
		clone.AddInt64("be:when", system_time());
		clone.AddPointer("be:source", static_cast<BHandler*>(this));
		clone.AddInt32("be:value", Value());
		clone.AddInt32("id", ID());
		return BInvoker::Invoke(&clone);
	}
	return BInvoker::Invoke(message);
}


bool
IconButton::IsValid() const
{
	return fNormalIcon != NULL && fDisabledIcon != NULL && fClickedIcon != NULL
		&& fDisabledClickedIcon != NULL
		&& !fNormalIcon->isNull()
		&& !fDisabledIcon->isNull()
		&& !fClickedIcon->isNull()
		&& !fDisabledClickedIcon->isNull();
}


bool
IconButton::HasRadioMode() const
{
	return isCheckable();
}


void
IconButton::SetRadioMode(bool radioMode)
{
	setCheckable(radioMode);
	setAutoExclusive(radioMode);

	disconnect(SIGNAL(toggled(bool)), this, SLOT(_CheckedChanged()));
	disconnect(SIGNAL(clicked()), this, SLOT(_Clicked()));

	if (radioMode)
		connect(this, SIGNAL(toggled(bool)), SLOT(_CheckedChanged()));
	else
		connect(this, SIGNAL(clicked()), SLOT(_Clicked()));
}


int32
IconButton::Value() const
{
	return isChecked() ? B_CONTROL_ON : B_CONTROL_OFF;
}


void
IconButton::SetValue(int32 value)
{
	SetPressed(value == B_CONTROL_ON);
}


void
IconButton::SetPressed(bool pressed)
{
	setChecked(pressed);
}


status_t
IconButton::SetIcon(int32 resourceID, int32 size)
{
	BResources resources;
	status_t status = get_app_resources(resources);
	if (status != B_OK)
		return status;

	size_t dataSize;
	const void* data = resources.LoadResource(B_VECTOR_ICON_TYPE, resourceID,
		&dataSize);
	if (data != NULL) {
		BBitmap bitmap(BRect(0, 0, size - 1, size - 1),
			B_BITMAP_NO_SERVER_LINK, B_RGBA32);
		status = bitmap.InitCheck();
		if (status != B_OK)
			return status;

		status = BIconUtils::GetVectorIcon(reinterpret_cast<const uint8*>(data),
			dataSize, &bitmap);
		if (status != B_OK)
			return status;
		return SetIcon(&bitmap);
	}
	return B_ERROR;
}


status_t
IconButton::SetIcon(const BBitmap* bitmap)
{
//	if (bitmap && bitmap->ColorSpace() == B_CMAP8) {
//		status_t status = bitmap->InitCheck();
//		if (status >= B_OK) {
//			if (BBitmap* rgb32Bitmap = _ConvertToRGB32(bitmap)) {
//				status = _MakeBitmaps(rgb32Bitmap);
//				delete rgb32Bitmap;
//			} else
//				status = B_NO_MEMORY;
//		}
//		return status;
//	} else
//		return _MakeBitmaps(bitmap);

setText(QString::fromUtf8("ToolButton"));
	status_t error =_MakeBitmaps(bitmap);
	_UpdateIcon();
	return error;
}


status_t
IconButton::SetIcon(const unsigned char* bitsFromQuickRes,
	uint32 width, uint32 height, color_space format, bool convertToBW)
{
	status_t status = B_BAD_VALUE;
	if (bitsFromQuickRes && width > 0 && height > 0) {
		BBitmap* quickResBitmap = new(std::nothrow) BBitmap(
			BRect(0.0, 0.0, width - 1.0, height - 1.0), format);
		status = quickResBitmap ? quickResBitmap->InitCheck() : B_ERROR;
		if (status >= B_OK) {
			// It doesn't look right to copy BitsLength() bytes, but bitmaps
			// exported from QuickRes still contain their padding, so it is alright.
			memcpy(quickResBitmap->Bits(), bitsFromQuickRes, quickResBitmap->BitsLength());
			if (format != B_RGB32 && format != B_RGBA32 && format != B_RGB32_BIG && format != B_RGBA32_BIG) {
				// colorspace needs conversion
				BBitmap* bitmap = new(std::nothrow) BBitmap(
					quickResBitmap->Bounds(), B_RGB32, true);
				if (bitmap && bitmap->IsValid()) {
// TODO:...
debugger("IconButton::SetIcon(): color space conversion not supported");
#if 0
					BView* helper = new BView(bitmap->Bounds(), "helper",
											  B_FOLLOW_NONE, B_WILL_DRAW);
					if (bitmap->Lock()) {
						bitmap->AddChild(helper);
						helper->SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
						helper->FillRect(helper->Bounds());
						helper->SetDrawingMode(B_OP_OVER);
						helper->DrawBitmap(quickResBitmap, BPoint(0.0, 0.0));
						helper->Sync();
						bitmap->Unlock();
					}
					status = _MakeBitmaps(bitmap);
#endif
				} else
					printf("IconButton::SetIcon() - B_RGB32 bitmap is not valid\n");
				delete bitmap;
			} else {
				// native colorspace (32 bits)
				if (convertToBW) {
					// convert to gray scale icon
					uint8* bits = (uint8*)quickResBitmap->Bits();
					uint32 bpr = quickResBitmap->BytesPerRow();
					for (uint32 y = 0; y < height; y++) {
						uint8* handle = bits;
						uint8 gray;
						for (uint32 x = 0; x < width; x++) {
							gray = uint8((116 * handle[0] + 600 * handle[1] + 308 * handle[2]) / 1024);
							handle[0] = gray;
							handle[1] = gray;
							handle[2] = gray;
							handle += 4;
						}
						bits += bpr;
					}
				}
				status = _MakeBitmaps(quickResBitmap);
			}
		} else
			printf("IconButton::SetIcon() - error allocating bitmap: %s\n", strerror(status));
		delete quickResBitmap;
	}
	return status;
}


void
IconButton::TrimIcon(BRect trimmed)
{
	if (fNormalBitmap == NULL)
		return;

	trimmed = trimmed & fNormalBitmap->Bounds();
	BBitmap trimmedBitmap(trimmed.OffsetToCopy(B_ORIGIN),
		B_BITMAP_NO_SERVER_LINK, B_RGBA32);
	uint8* bits = (uint8*)fNormalBitmap->Bits();
	uint32 bpr = fNormalBitmap->BytesPerRow();
	bits += 4 * (int32)trimmed.left + bpr * (int32)trimmed.top;
	uint8* dst = (uint8*)trimmedBitmap.Bits();
	uint32 trimmedWidth = trimmedBitmap.Bounds().IntegerWidth() + 1;
	uint32 trimmedHeight = trimmedBitmap.Bounds().IntegerHeight() + 1;
	uint32 trimmedBPR = trimmedBitmap.BytesPerRow();
	for (uint32 y = 0; y < trimmedHeight; y++) {
		memcpy(dst, bits, trimmedWidth * 4);
		dst += trimmedBPR;
		bits += bpr;
	}
	SetIcon(&trimmedBitmap);
}


status_t
IconButton::_MakeBitmaps(const BBitmap* bitmap)
{
	status_t status = bitmap ? bitmap->InitCheck() : B_BAD_VALUE;
	if (status >= B_OK) {
		// make our own versions of the bitmap
		BRect b(bitmap->Bounds());
		_DeleteBitmaps();
		color_space format = bitmap->ColorSpace();
		fNormalBitmap = new(std::nothrow) BBitmap(b, format);
		BBitmap disabledBitmap(b, format);
		BBitmap clickedBitmap(b, format);
		BBitmap disabledClickedBitmap(b, format);
		if (fNormalBitmap != NULL && fNormalBitmap->IsValid()
			&& disabledBitmap.IsValid() && clickedBitmap.IsValid()
			&& disabledBitmap.IsValid()) {
			// copy bitmaps from file bitmap
			uint8* nBits = (uint8*)fNormalBitmap->Bits();
			uint8* dBits = (uint8*)disabledBitmap.Bits();
			uint8* cBits = (uint8*)clickedBitmap.Bits();
			uint8* dcBits = (uint8*)disabledClickedBitmap.Bits();
			uint8* fBits = (uint8*)bitmap->Bits();
			int32 nbpr = fNormalBitmap->BytesPerRow();
			int32 fbpr = bitmap->BytesPerRow();
			int32 pixels = b.IntegerWidth() + 1;
			int32 lines = b.IntegerHeight() + 1;
			// nontransparent version:
			if (format == B_RGB32 || format == B_RGB32_BIG) {
				// iterate over color components
				for (int32 y = 0; y < lines; y++) {
					for (int32 x = 0; x < pixels; x++) {
						int32 nOffset = 4 * x;
						int32 fOffset = 4 * x;
						nBits[nOffset + 0] = fBits[fOffset + 0];
						nBits[nOffset + 1] = fBits[fOffset + 1];
						nBits[nOffset + 2] = fBits[fOffset + 2];
						nBits[nOffset + 3] = 255;
						// clicked bits are darker (lame method...)
						cBits[nOffset + 0] = (uint8)((float)nBits[nOffset + 0] * 0.8);
						cBits[nOffset + 1] = (uint8)((float)nBits[nOffset + 1] * 0.8);
						cBits[nOffset + 2] = (uint8)((float)nBits[nOffset + 2] * 0.8);
						cBits[nOffset + 3] = 255;
						// disabled bits have less contrast (lame method...)
						uint8 grey = 216;
						float dist = (nBits[nOffset + 0] - grey) * 0.4;
						dBits[nOffset + 0] = (uint8)(grey + dist);
						dist = (nBits[nOffset + 1] - grey) * 0.4;
						dBits[nOffset + 1] = (uint8)(grey + dist);
						dist = (nBits[nOffset + 2] - grey) * 0.4;
						dBits[nOffset + 2] = (uint8)(grey + dist);
						dBits[nOffset + 3] = 255;
						// disabled bits have less contrast (lame method...)
						grey = 188;
						dist = (nBits[nOffset + 0] - grey) * 0.4;
						dcBits[nOffset + 0] = (uint8)(grey + dist);
						dist = (nBits[nOffset + 1] - grey) * 0.4;
						dcBits[nOffset + 1] = (uint8)(grey + dist);
						dist = (nBits[nOffset + 2] - grey) * 0.4;
						dcBits[nOffset + 2] = (uint8)(grey + dist);
						dcBits[nOffset + 3] = 255;
					}
					nBits += nbpr;
					dBits += nbpr;
					cBits += nbpr;
					dcBits += nbpr;
					fBits += fbpr;
				}
			// transparent version:
			} else if (format == B_RGBA32 || format == B_RGBA32_BIG) {
				// iterate over color components
				for (int32 y = 0; y < lines; y++) {
					for (int32 x = 0; x < pixels; x++) {
						int32 nOffset = 4 * x;
						int32 fOffset = 4 * x;
						nBits[nOffset + 0] = fBits[fOffset + 0];
						nBits[nOffset + 1] = fBits[fOffset + 1];
						nBits[nOffset + 2] = fBits[fOffset + 2];
						nBits[nOffset + 3] = fBits[fOffset + 3];
						// clicked bits are darker (lame method...)
						cBits[nOffset + 0] = (uint8)(nBits[nOffset + 0] * 0.8);
						cBits[nOffset + 1] = (uint8)(nBits[nOffset + 1] * 0.8);
						cBits[nOffset + 2] = (uint8)(nBits[nOffset + 2] * 0.8);
						cBits[nOffset + 3] = fBits[fOffset + 3];
						// disabled bits have less opacity

						uint8 grey = ((uint16)nBits[nOffset + 0] * 10
							+ nBits[nOffset + 1] * 60
							+ nBits[nOffset + 2] * 30) / 100;
						float dist = (nBits[nOffset + 0] - grey) * 0.3;
						dBits[nOffset + 0] = (uint8)(grey + dist);
						dist = (nBits[nOffset + 1] - grey) * 0.3;
						dBits[nOffset + 1] = (uint8)(grey + dist);
						dist = (nBits[nOffset + 2] - grey) * 0.3;
						dBits[nOffset + 2] = (uint8)(grey + dist);
						dBits[nOffset + 3] = (uint8)(fBits[fOffset + 3] * 0.3);
						// disabled bits have less contrast (lame method...)
						dcBits[nOffset + 0] = (uint8)(dBits[nOffset + 0] * 0.8);
						dcBits[nOffset + 1] = (uint8)(dBits[nOffset + 1] * 0.8);
						dcBits[nOffset + 2] = (uint8)(dBits[nOffset + 2] * 0.8);
						dcBits[nOffset + 3] = (uint8)(fBits[fOffset + 3] * 0.3);
					}
					nBits += nbpr;
					dBits += nbpr;
					cBits += nbpr;
					dcBits += nbpr;
					fBits += fbpr;
				}
			// unsupported format
			} else {
				printf("IconButton::_MakeBitmaps() - bitmap has unsupported colorspace\n");
				status = B_MISMATCHED_VALUES;
				_DeleteBitmaps();
			}
		} else {
			printf("IconButton::_MakeBitmaps() - error allocating local bitmaps\n");
			status = B_NO_MEMORY;
			_DeleteBitmaps();
		}

		if (status == B_OK) {
			fNormalIcon = new(std::nothrow) QIcon(fNormalBitmap->ToQIcon());
			fDisabledIcon = new(std::nothrow) QIcon(disabledBitmap.ToQIcon());
			fClickedIcon = new(std::nothrow) QIcon(clickedBitmap.ToQIcon());
			fDisabledClickedIcon
				= new(std::nothrow) QIcon(disabledClickedBitmap.ToQIcon());

			if (fNormalIcon == NULL || fDisabledIcon == NULL
				|| fClickedIcon == NULL || fDisabledClickedIcon == NULL) {
				status = B_NO_MEMORY;
				_DeleteBitmaps();
			}
		}
	} else
		printf("IconButton::_MakeBitmaps() - bitmap is not valid\n");

	return status;
}


void
IconButton::_DeleteBitmaps()
{
	delete fNormalBitmap;
	fNormalBitmap = NULL;
	delete fNormalIcon;
	fNormalIcon = NULL;
	delete fDisabledIcon;
	fDisabledIcon = NULL;
	delete fClickedIcon;
	fClickedIcon = NULL;
	delete fDisabledClickedIcon;
	fDisabledClickedIcon = NULL;
}


void IconButton::_CheckedChanged()
{
	if (isChecked() && isEnabled())
		Invoke();
}


void
IconButton::_Clicked()
{
	if (isEnabled())
		Invoke();
}


void
IconButton::_UpdateIcon()
{
	QIcon* icon;
	if (isEnabled())
		icon = isDown() ? fClickedIcon : fNormalIcon;
	else
		icon = isDown() ? fDisabledClickedIcon : fDisabledIcon;

	if (icon != NULL)
		setIcon(*icon);
}
