/*
 * Copyright 2001 Werner Freytag - please read to the LICENSE file
 *
 * Copyright 2002-2006, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Application.h>
#include <Bitmap.h>
#include <Font.h>
#include <Message.h>
#include <MessageRunner.h>
#include <Screen.h>
#include <Window.h>

#include "ColorField.h"
#include "ColorPreview.h"
#include "ColorSlider.h"
#include "rgb_hsv.h"

#include "ColorPickerView.h"
#include "ColorPickerViewPlatformDelegate.h"


#define round(x) (int)(x+.5)
#define hexdec(str, offset) (int)(((str[offset]<60?str[offset]-48:(str[offset]|32)-87)<<4)|(str[offset+1]<60?str[offset+1]-48:(str[offset+1]|32)-87))

// constructor
ColorPickerView::ColorPickerView(const char* name, rgb_color color,
								 SelectedColorMode mode)
	: PlatformViewMixin<BView>(name, 0),
	  fPlatformDelegate(new PlatformDelegate(this)),
	  h(0.0),
	  s(1.0),
	  v(1.0),
	  r((float)color.red / 255.0),
	  g((float)color.green / 255.0),
	  b((float)color.blue / 255.0),
	  fRequiresUpdate(false)
{
	RGB_to_HSV(r, g, b, h, s, v);

	SetColorMode(mode, false);
}

// destructor
ColorPickerView::~ColorPickerView()
{
	delete fPlatformDelegate;
}

// AttachedToWindow
void
ColorPickerView::AttachedToWindow()
{
	rgb_color	color = { (int)(r * 255), (int)(g * 255), (int)(b * 255), 255 };

	BView::AttachedToWindow();

	fColorField = new ColorField(fSelectedColorMode, *p);
	fColorField->SetMarkerToColor(color);
	fColorField->SetTarget(this);

	fColorSlider = new ColorSlider(fSelectedColorMode, *p1, *p2);
	fColorSlider->SetMarkerToColor(color);
	fColorSlider->SetTarget(this);

	fColorPreview = new ColorPreview(color);
	fColorPreview->SetTarget(this);

	fPlatformDelegate->Init(_NumForMode(fSelectedColorMode));

	_UpdateTextControls();
}

// MessageReceived
void
ColorPickerView::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case MSG_UPDATE_COLOR_PICKER_VIEW:
			if (fRequiresUpdate)
				_UpdateTextControls();
			break;

		case MSG_COLOR_FIELD: {
			float	value1, value2;
			value1 = message->FindFloat("value");
			value2 = message->FindFloat("value", 1);
			_UpdateColor(-1, value1, value2);
			_UpdateTextControls();
		} break;

		case MSG_COLOR_SLIDER: {
			float	value;
			message->FindFloat("value", &value);
			_UpdateColor(value, -1, -1);
			_UpdateTextControls();
		} break;

		case MSG_COLOR_PREVIEW: {
			rgb_color	*color;
			ssize_t		numBytes;
			if (message->FindData("color", B_RGB_COLOR_TYPE, (const void **)&color, &numBytes)==B_OK) {
				color->alpha = 255;
				SetColor(*color);
			}
		} break;

		case MSG_RADIOBUTTON: {
			SetColorMode(H_SELECTED);
		} break;

		case MSG_RADIOBUTTON + 1: {
			SetColorMode(S_SELECTED);
		} break;

		case MSG_RADIOBUTTON + 2: {
			SetColorMode(V_SELECTED);
		} break;

		case MSG_RADIOBUTTON + 3: {
			SetColorMode(R_SELECTED);
		} break;

		case MSG_RADIOBUTTON + 4: {
			SetColorMode(G_SELECTED);
		} break;

		case MSG_RADIOBUTTON + 5: {
			SetColorMode(B_SELECTED);
		} break;

		case MSG_TEXTCONTROL:
		case MSG_TEXTCONTROL + 1:
		case MSG_TEXTCONTROL + 2:
		case MSG_TEXTCONTROL + 3:
		case MSG_TEXTCONTROL + 4:
		case MSG_TEXTCONTROL + 5: {

			int nr = message->what - MSG_TEXTCONTROL;
			int value = fPlatformDelegate->TextControlValue(nr);

			switch (nr) {
				case 0: {
					value %= 360;
					h = (float)value / 60;
				} break;

				case 1: {
					value = min_c(value, 100);
					s = (float)value / 100;
				} break;

				case 2: {
					value = min_c(value, 100);
					v = (float)value / 100;
				} break;

				case 3: {
					value = min_c(value, 255);
					r = (float)value / 255;
				} break;

				case 4: {
					value = min_c(value, 255);
					g = (float)value / 255;
				} break;

				case 5: {
					value = min_c(value, 255);
					b = (float)value / 255;
				} break;
			}

			if (nr<3) { // hsv-mode
				HSV_to_RGB(h, s, v, r, g, b);
			}

			rgb_color color = { round(r*255), round(g*255), round(b*255), 255 };

			SetColor(color);

		} break;

		case MSG_HEXTEXTCONTROL: {
			BString string = fPlatformDelegate->HexTextControlString();
			if (string.Length() == 6) {
				rgb_color color = { hexdec(string, 0), hexdec(string, 2),
					hexdec(string, 4), 255 };
				SetColor(color);
			}
		} break;

		default:
			BView::MessageReceived(message);
	}
}

// PlatformDraw
void
ColorPickerView::PlatformDraw(PlatformDrawContext& drawContext)
{
	fPlatformDelegate->Draw(drawContext);
}

// SetColorMode
void
ColorPickerView::SetColorMode(SelectedColorMode mode, bool update)
{
	fSelectedColorMode = mode;
	switch (mode) {
		case R_SELECTED:
			p = &r; p1 = &g; p2 = &b;
		break;

		case G_SELECTED:
			p = &g; p1 = &r; p2 = &b;
		break;

		case B_SELECTED:
			p = &b; p1 = &r; p2 = &g;
		break;

		case H_SELECTED:
			p = &h; p1 = &s; p2 = &v;
		break;

		case S_SELECTED:
			p = &s; p1 = &h; p2 = &v;
		break;

		case V_SELECTED:
			p = &v; p1 = &h; p2 = &s;
		break;
	}

	if (!update) return;

	fColorSlider->SetModeAndValues(fSelectedColorMode, *p1, *p2);
	fColorField->SetModeAndValue(fSelectedColorMode, *p);

}

// SetColor
void
ColorPickerView::SetColor(rgb_color color)
{
	r = (float)color.red/255; g = (float)color.green/255; b = (float)color.blue/255;
	RGB_to_HSV(r, g, b, h, s, v);

	fColorSlider->SetModeAndValues(fSelectedColorMode, *p1, *p2);
	fColorSlider->SetMarkerToColor(color);

	fColorField->SetModeAndValue(fSelectedColorMode, *p);
	fColorField->SetMarkerToColor(color);

	fColorPreview->SetColor(color);

	_UpdateTextControls();
}

// Color
rgb_color
ColorPickerView::Color()
{
	if (fSelectedColorMode & (R_SELECTED|G_SELECTED|B_SELECTED))
		RGB_to_HSV(r, g, b, h, s, v);
	else
		HSV_to_RGB(h, s, v, r, g, b);

	rgb_color color;
	color.red = (uint8)round(r * 255.0);
	color.green = (uint8)round(g * 255.0);
	color.blue = (uint8)round(b * 255.0);
	color.alpha = 255;

	return color;
}

// _NumForMode
int32
ColorPickerView::_NumForMode(SelectedColorMode mode) const
{
	int32 num = -1;
	switch (mode) {
		case H_SELECTED:
			num = 0;
			break;
		case S_SELECTED:
			num = 1;
			break;
		case V_SELECTED:
			num = 2;
			break;
		case R_SELECTED:
			num = 3;
			break;
		case G_SELECTED:
			num = 4;
			break;
		case B_SELECTED:
			num = 5;
			break;
	}
	return num;
}

// _UpdateColor
void
ColorPickerView::_UpdateColor(float value, float value1, float value2)
{
	if (value!=-1) {
		fColorField->SetFixedValue(value);
		*p = value;
	}
	else if (value1!=-1 && value2!=-1) {
		fColorSlider->SetOtherValues(value1, value2);
		*p1 = value1; *p2 = value2;
	}

	if (fSelectedColorMode & (R_SELECTED|G_SELECTED|B_SELECTED))
		RGB_to_HSV(r, g, b, h, s, v);
	else
		HSV_to_RGB(h, s, v, r, g, b);

	rgb_color color = { (int)(r*255), (int)(g*255), (int)(b*255), 255 };
	fColorPreview->SetColor(color);
}

// _UpdateTextControls
void
ColorPickerView::_UpdateTextControls()
{
	bool updateRequired = false;
	updateRequired |= fPlatformDelegate->SetTextControlValue(0, round(h * 60));
	updateRequired |= fPlatformDelegate->SetTextControlValue(1, round(s * 100));
	updateRequired |= fPlatformDelegate->SetTextControlValue(2, round(v * 100));
	updateRequired |= fPlatformDelegate->SetTextControlValue(3, round(r * 255));
	updateRequired |= fPlatformDelegate->SetTextControlValue(4, round(g * 255));
	updateRequired |= fPlatformDelegate->SetTextControlValue(5, round(b * 255));

	BString hexString;
	hexString.SetToFormat("%.6X",
		(round(r * 255) << 16) | (round(g * 255) << 8) | round(b * 255));
	updateRequired |= fPlatformDelegate->SetHexTextControlString(hexString);

	fRequiresUpdate = updateRequired;
	if (fRequiresUpdate) {
		// Couldn't set all values. Try again later.
		BMessage message(MSG_UPDATE_COLOR_PICKER_VIEW);
		BMessageRunner::StartSending(this, &message, 500000, 1);
	}
}
