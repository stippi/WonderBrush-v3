/*
 * Copyright 2001 Werner Freytag - please read to the LICENSE file
 *
 * Copyright 2002-2006, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 *
 */
#ifndef COLOR_PICKER_VIEW_PLATFORM_DELEGATE_H
#define COLOR_PICKER_VIEW_PLATFORM_DELEGATE_H

#include <ControlLook.h>
#include <Font.h>
#include <GroupLayout.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <RadioButton.h>
#include <Screen.h>
#include <String.h>
#include <StringView.h>
#include <TextControl.h>

#include "ColorPickerView.h"


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
	}


	void Init(int32 selectedRadioButton)
	{
		fView->fColorField->SetExplicitMinSize(BSize(256, 256));
		fView->fColorField->SetExplicitMaxSize(
			BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));
		fView->fColorSlider->SetExplicitMaxSize(
			BSize(B_SIZE_UNSET, B_SIZE_UNLIMITED));
		fView->fColorPreview->SetExplicitMinSize(BSize(B_SIZE_UNSET, 70));
		
		const char* title[] = { "H", "S", "V", "R", "G", "B" };

		for (int i = 0; i < 6; i++) {
			fRadioButton[i] = new BRadioButton(NULL, title[i],
				new BMessage(MSG_RADIOBUTTON + i));
			fRadioButton[i]->SetTarget(fView);

			if (i == selectedRadioButton)
				fRadioButton[i]->SetValue(1);

			fTextControl[i] = new BTextControl(NULL, NULL, NULL,
				new BMessage(MSG_TEXTCONTROL + i));

			fTextControl[i]->TextView()->SetMaxBytes(3);
			for (int j = 32; j < 255; ++j) {
				if (j < '0' || j > '9')
					fTextControl[i]->TextView()->DisallowChar(j);
			}
		}

		fHexTextControl = new BTextControl(NULL, "#", NULL,
			new BMessage(MSG_HEXTEXTCONTROL));

		fHexTextControl->TextView()->SetMaxBytes(6);
		for (int j = 32; j < 255; ++j) {
			if (!((j >= '0' && j <= '9') || (j >= 'a' && j <= 'f')
				|| (j >= 'A' && j <= 'F'))) {
				fHexTextControl->TextView()->DisallowChar(j);
			}
		}

		const float inset = be_control_look->DefaultLabelSpacing();
		BSize separatorSize(B_SIZE_UNSET, inset / 2);
		BAlignment separatorAlignment(B_ALIGN_LEFT, B_ALIGN_TOP);

		fView->SetLayout(new BGroupLayout(B_HORIZONTAL));
		BLayoutBuilder::Group<>(fView, B_HORIZONTAL)
			.AddGroup(B_HORIZONTAL, 0.0f)
				.Add(fView->fColorField)
			.SetInsets(3, 3, 0, 3)
			.End()
			.Add(fView->fColorSlider)
			.AddGroup(B_VERTICAL)
				.Add(fView->fColorPreview)
				.AddGrid(inset / 2, inset / 2)
					.Add(fRadioButton[0], 0, 0)
					.Add(fTextControl[0], 1, 0)
					.Add(new BStringView(NULL, "°"), 2, 0)

					.Add(fRadioButton[1], 0, 1)
					.Add(fTextControl[1], 1, 1)
					.Add(new BStringView(NULL, "%"), 2, 1)

					.Add(fRadioButton[2], 0, 2)
					.Add(fTextControl[2], 1, 2)
					.Add(new BStringView(NULL, "%"), 2, 2)

					.Add(new BSpaceLayoutItem(separatorSize, separatorSize,
						separatorSize, separatorAlignment),
						0, 3, 3)

					.Add(fRadioButton[3], 0, 4)
					.Add(fTextControl[3], 1, 4)

					.Add(fRadioButton[4], 0, 5)
					.Add(fTextControl[4], 1, 5)

					.Add(fRadioButton[5], 0, 6)
					.Add(fTextControl[5], 1, 6)

					.Add(new BSpaceLayoutItem(separatorSize, separatorSize,
						separatorSize, separatorAlignment),
						0, 7, 3)
					
					.AddGroup(B_HORIZONTAL, 0.0f, 0, 8, 2)
						.Add(fHexTextControl->CreateLabelLayoutItem())
						.Add(fHexTextControl->CreateTextViewLayoutItem())
					.End()
				.End()
			.SetInsets(0, 3, 3, 3)
			.End()
			.SetInsets(inset, inset, inset, inset)
		;

		// After the views are attached, configure their target
		for (int i = 0; i < 6; i++) {
			fRadioButton[i]->SetTarget(fView);
			fTextControl[i]->SetTarget(fView);
		}
		fHexTextControl->SetTarget(fView);
	}

	void Draw(PlatformDrawContext& drawContext)
	{
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
