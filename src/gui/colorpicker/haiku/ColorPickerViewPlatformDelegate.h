/*
 * Copyright 2001 Werner Freytag - please read to the LICENSE file
 *
 * Copyright 2002-2006, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 *
 */
#ifndef COLOR_PICKER_VIEW_PLATFORM_DELEGATE_H
#define COLOR_PICKER_VIEW_PLATFORM_DELEGATE_H


#include <Font.h>
#include <Message.h>
#include <RadioButton.h>
#include <Screen.h>
#include <TextControl.h>


#include "ColorPickerView.h"
#include "PlatformSignalMessageAdapter.h"


class ColorPickerView::PlatformDelegate {
public:
	PlatformDelegate(ColorPickerView* view)
		:
		fView(view)
	{
		fView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	}

	PlatformDelegate()
	{
		for (int i = 0; i < 6; i++)
			fTextControlAdapters[i].DisconnectAll();
		fHexTextControlAdapter.DisconnectAll();
	}


	void Init(int32 selectedRadioButton)
	{
		fView->AddChild(fView->fColorField);
		AddChild(fView->fColorSlider);
		AddChild(fView->fColorPreview);

		BFont font(be_plain_font);
		font.SetSize(10.0);
		fView->SetFont(&font);

		const char *title[] = { "H", "S", "V", "R", "G", "B" };

		BTextView	*textView;

		for (int i=0; i<6; ++i) {

			fRadioButton[i] = new BRadioButton(
				BRect(0.0, 0.0, 30.0, 10.0)
					.OffsetToCopy(320.0, 92.0 + 24.0 * i + (int)i/3 * 8),
				NULL, title[i], new BMessage(MSG_RADIOBUTTON + i));
			fRadioButton[i]->SetFont(&font);
			fView->AddChild(fRadioButton[i]);

			fRadioButton[i]->SetTarget(fView);

			if (i == selectedRadioButton)
				fRadioButton[i]->SetValue(1);
		}

		for (int i=0; i<6; ++i) {

			fTextControl[i] = new BTextControl(
				BRect(0.0, 0.0, 32.0, 19.0)
					.OffsetToCopy(350.0, 90.0 + 24.0 * i + (int)i/3 * 8),
				NULL, NULL, NULL, new BMessage(MSG_TEXTCONTROL + i));

			textView = fTextControl[i]->TextView();
			textView->SetMaxBytes(3);
			for (int j=32; j<255; ++j) {
				if (j<'0'||j>'9') textView->DisallowChar(j);
			}

			fTextControl[i]->SetFont(&font);
			fTextControl[i]->SetDivider(0.0);
			fView->AddChild(fTextControl[i]);

			fTextControl[i]->SetTarget(fView);
		}

		fHexTextControl = new BTextControl(
			BRect(0.0, 0.0, 69.0, 19.0).OffsetToCopy(320.0, 248.0),
			NULL, "#", NULL, new BMessage(MSG_HEXTEXTCONTROL));

		textView = fHexTextControl->TextView();
		textView->SetMaxBytes(6);
		for (int j=32; j<255; ++j) {
			if (!((j>='0' && j<='9') || (j>='a' && j<='f')
				|| (j>='A' && j<='F'))) {
				textView->DisallowChar(j);
			}
		}

		fHexTextControl->SetFont(&font);
		fHexTextControl->SetDivider(12.0);
		fView->AddChild(fHexTextControl);

		fHexTextControl->SetTarget(fView);
	}

	void Draw(PlatformDrawContext& drawContext)
	{
		BView* view = drawContext.View();
		BRect updateRect = drawContext.UpdateRect();

		// raised border
		BRect r(view->Bounds());
		if (updateRect.Intersects(r)) {
			rgb_color light = tint_color(view->LowColor(), B_LIGHTEN_MAX_TINT);
			rgb_color shadow = tint_color(view->LowColor(), B_DARKEN_2_TINT);

			view->BeginLineArray(4);
				view->AddLine(BPoint(r.left, r.bottom),
						BPoint(r.left, r.top), light);
				view->AddLine(BPoint(r.left + 1.0, r.top),
						BPoint(r.right, r.top), light);
				view->AddLine(BPoint(r.right, r.top + 1.0),
						BPoint(r.right, r.bottom), shadow);
				view->AddLine(BPoint(r.right - 1.0, r.bottom),
						BPoint(r.left + 1.0, r.bottom), shadow);
			view->EndLineArray();
			// exclude border from update rect
			r.InsetBy(1.0, 1.0);
			updateRect = r & updateRect;
		}

		// some additional labels
		font_height fh;
		view->GetFontHeight(&fh);

		const char *title[] = { "°", "%", "%" };
		for (int i = 0; i < 3; ++i) {
			view->DrawString(title[i],
				BPoint(385.0, 93.0 + 24.0 * i + (int)i / 3 * 8 + fh.ascent));
		}
	}

	int TextControlValue(int32 index)
	{
		return atoi(fTextControl[index]->Text());
	}

	// Returns whether the value needs to be set later, since it is currently
	// being edited by the user.
	bool SetTextControlValue(int32 index, int value)
	{
		BString text;
		text << value;
		return _SetText(fTextControl[index], text);
	}

	BString HexTextControlString() const
	{
		return fHexTextControl->TextView()->Text();
	}

	// Returns whether the value needs to be set later, since it is currently
	// being edited by the user.
	bool SetHexTextControlString(const BString& text)
	{
		return _SetText(fHexTextControl, text);
	}

private:
	// Returns whether the value needs to be set later, since it is currently
	// being edited by the user.
	bool _SetText(BTextControl* control, const BString& text)
	{
		if (text == control->Text())
			return false;

		// This textview needs updating, but don't screw with user while she is
		// typing.
		if (control->TextView()->IsFocus())
			return true;

		control->SetText(text.String());
		return false;
	}

private:
	ColorPickerView*	fView;
	BRadioButton*		fRadioButton[6];
	BTextControl*		fTextControl[6];
	BTextControl*		fHexTextControl;
};


#endif // COLOR_PICKER_VIEW_PLATFORM_DELEGATE_H
