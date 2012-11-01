/*
 * Copyright 2006-2012 Stephan Aßmus <superstippi@gmx.de>
 * Distributed under the terms of the MIT License.
 */

#include "SwatchGroup.h"

#include <stdio.h>

#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>

#include "support_ui.h"
#include "ui_defines.h"
#include "rgb_hsv.h"

#include "AlphaSlider.h"
#include "ColorField.h"
#include "ColorPickerPanel.h"
#include "ColorSlider.h"
//#include "CurrentColor.h"
#include "SwatchView.h"


enum {
	MSG_SET_COLOR		= 'stcl',
	MSG_COLOR_PICKER	= 'clpk',
	MSG_ALPHA_SLIDER	= 'alps',
};


#define SWATCH_VIEW_WIDTH 20
#define SWATCH_VIEW_HEIGHT 15


SwatchGroup::SwatchGroup(const char* name)
	: BView(name, 0)
//	, fCurrentColor(NULL)
	, fIgnoreNotifications(false)

	, fColorPickerPanel(NULL)
	, fColorPickerMode(H_SELECTED)
	, fColorPickerFrame(100.0, 100.0, 200.0, 200.0)
{
	BGroupLayout* layout = new BGroupLayout(B_VERTICAL);
	SetLayout(layout);

	// create swatch views with rainbow default palette
	float h = 0;
	float s = 1.0;
	float v = 1.0;
	rgb_color color;
	color.alpha = 255;
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	for (int32 i = 0; i < 20; i++) {
		if (i < 10) {
			h = ((float)i / 9.0) * 6.0;
		} else {
			h = ((float)(i - 9) / 10.0) * 6.0;
			v = 0.5;
		}

		HSV_to_RGB(h, s, v, r, g, b);
		color.red = (uint8)(255.0 * r);
		color.green = (uint8)(255.0 * g);
		color.blue = (uint8)(255.0 * b);
		fSwatchViews[i] = new SwatchView("swatch", new BMessage(MSG_SET_COLOR),
			this, color, SWATCH_VIEW_WIDTH, SWATCH_VIEW_HEIGHT);
	}

	// create current color swatch view
	fCurrentColorSV = new SwatchView("current swatch",
		new BMessage(MSG_COLOR_PICKER), this, color, 28.0, 28.0);

	// When the color of this swatch changes via drag&drop, we want to
	// adopt it as current color.
	fCurrentColorSV->SetDroppedMessage(new BMessage(MSG_SET_COLOR));

	// create color field and slider
	fColorField = new ColorField(H_SELECTED, 1.0, B_HORIZONTAL);
	fColorSlider = new ColorSlider(H_SELECTED, 1.0, 1.0, B_HORIZONTAL);
	fAlphaSlider = new AlphaSlider(B_HORIZONTAL,
		new BMessage(MSG_ALPHA_SLIDER));

	// layout UI
	BGroupLayoutBuilder(layout)
		.AddGroup(B_HORIZONTAL, 0.0f)
			.Add(fCurrentColorSV)
			.AddGroup(B_VERTICAL, 0.0f)
				.AddGroup(B_HORIZONTAL, 0.0f)
					.Add(fSwatchViews[0])
					.Add(fSwatchViews[1])
					.Add(fSwatchViews[2])
					.Add(fSwatchViews[3])
					.Add(fSwatchViews[4])
					.Add(fSwatchViews[5])
					.Add(fSwatchViews[6])
					.Add(fSwatchViews[7])
					.Add(fSwatchViews[8])
					.Add(fSwatchViews[9])
				.End()
				.AddGroup(B_HORIZONTAL, 0.0f)
					.Add(fSwatchViews[10])
					.Add(fSwatchViews[11])
					.Add(fSwatchViews[12])
					.Add(fSwatchViews[13])
					.Add(fSwatchViews[14])
					.Add(fSwatchViews[15])
					.Add(fSwatchViews[16])
					.Add(fSwatchViews[17])
					.Add(fSwatchViews[18])
					.Add(fSwatchViews[19])
				.End()
			.End()
		.End()
		.Add(fColorField)
		.Add(fColorSlider)
		.Add(fAlphaSlider)
		.SetInsets(0, 0, 0, 0)
	;
}


SwatchGroup::~SwatchGroup()
{
//	SetCurrentColor(NULL);
}


void
SwatchGroup::ObjectChanged(const Notifier* object)
{
//	if (object != fCurrentColor || fIgnoreNotifications)
//		return;
//
//	rgb_color color = fCurrentColor->Color();
//
//	float h, s, v;
//	RGB_to_HSV(color.red / 255.0, color.green / 255.0, color.blue / 255.0,
//		h, s, v);
//
//	_SetColor(h, s, v, color.alpha);
}


// #pragma mark -


void
SwatchGroup::AttachedToWindow()
{
	fColorField->SetTarget(this);
	fColorSlider->SetTarget(this);
	fAlphaSlider->SetTarget(this);
}


void
SwatchGroup::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_SET_COLOR:
		{
			rgb_color color;
			if (restore_color_from_message(message, color) == B_OK) {
// TODO: fix color picker panel to respect alpha
if (message->HasRect("panel frame"))
color.alpha = fAlphaSlider->Value();
//
//				if (fCurrentColor != NULL)
//					fCurrentColor->SetColor(color);
			}
			// if message contains these fields,
			// then it comes from the color picker panel.
			// it also means the panel has died.
			BRect frame;
			SelectedColorMode mode;
			if (message->FindRect("panel frame", &frame) == B_OK
				&& message->FindInt32("panel mode", (int32*)&mode) == B_OK) {
				// message came from the color picker panel
				// we remember the settings of the panel for later
				fColorPickerFrame = frame;
				fColorPickerMode = mode;
				// color picker panel has quit
				fColorPickerPanel = NULL;
			}
			break;
		}

		case MSG_COLOR_FIELD:
		{
			// get h from color slider
			float h = ((255 - fColorSlider->Value()) / 255.0) * 6.0;
			float s, v;
			// s and v are comming from the message
			if (message->FindFloat("value", &s) == B_OK
				&& message->FindFloat("value", 1, &v) == B_OK) {
				_SetColor(h, s, v, fAlphaSlider->Value());
			}
			break;
		}

		case MSG_COLOR_SLIDER:
		{
			float h;
			float s, v;
			fColorSlider->GetOtherValues(&s, &v);
			// h is comming from the message
			if (message->FindFloat("value", &h) == B_OK)
				_SetColor(h, s, v, fAlphaSlider->Value());
			break;
		}

		case MSG_ALPHA_SLIDER:
		{
			float h = (1.0 - (float)fColorSlider->Value() / 255.0) * 6;
			float s, v;
			fColorSlider->GetOtherValues(&s, &v);
			_SetColor(h, s, v, fAlphaSlider->Value());
			break;
		}

		case MSG_COLOR_PICKER:
		{
			rgb_color color;
			if (restore_color_from_message(message, color) < B_OK)
				break;

			if (fColorPickerPanel == NULL) {
				fColorPickerPanel = new ColorPickerPanel(fColorPickerFrame,
					color, fColorPickerMode, Window(),
					new BMessage(MSG_SET_COLOR), this);
				fColorPickerPanel->Show();
			} else {
				if (fColorPickerPanel->Lock()) {
					fColorPickerPanel->SetColor(color);
					fColorPickerPanel->Activate();
					fColorPickerPanel->Unlock();
				}
			}
			break;
		}

		default:
			BView::MessageReceived(message);
			break;
	}
}


// #pragma mark -


//void
//SwatchGroup::SetCurrentColor(CurrentColor* color)
//{
//	if (fCurrentColor == color)
//		return;
//
//	if (fCurrentColor != NULL)
//		fCurrentColor->RemoveObserver(this);
//
//	fCurrentColor = color;
//
//	if (fCurrentColor != NULL) {
//		fCurrentColor->AddObserver(this);
//
//		ObjectChanged(fCurrentColor);
//	}
//}


// #pragma mark -


void
SwatchGroup::_SetColor(rgb_color color)
{
	fCurrentColorSV->SetColor(color);
}


void
SwatchGroup::_SetColor(float h, float s, float v, uint8 a)
{
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	HSV_to_RGB(h, s, v, r, g, b);

	rgb_color color;
	color.red = (uint8)(r * 255.0);
	color.green = (uint8)(g * 255.0);
	color.blue = (uint8)(b * 255.0);
	color.alpha = a;

	if (!fColorField->IsTracking()) {
		fColorField->SetFixedValue(h);
		fColorField->SetMarkerToColor(color);
	}
	if (!fColorSlider->IsTracking()) {
		fColorSlider->SetOtherValues(s, v);
		fColorSlider->SetValue(255 - (int32)((h / 6.0) * 255.0 + 0.5));
	}
	if (!fAlphaSlider->IsTracking()) {
		fAlphaSlider->SetColor(color);
		fAlphaSlider->SetValue(a);
	}

	fIgnoreNotifications = true;

//	if (fCurrentColor)
//		fCurrentColor->SetColor(color);
	_SetColor(color);

	fIgnoreNotifications = false;
}
