/*
 * Copyright 2002-2006, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */
#ifndef COLOR_PICKER_PANEL_PLATFORM_DELEGATE_H
#define COLOR_PICKER_PANEL_PLATFORM_DELEGATE_H


#include <Box.h>
#include <Button.h>

#include "ColorPickerPanel.h"
#include "ColorPickerView.h"


class ColorPickerPanel::PlatformDelegate {
public:
	PlatformDelegate(ColorPickerPanel* panel)
		:
		fPanel(panel)
	{
	}

	void Init()
	{
		BRect frame = BRect(0, 0, 40, 15);
		BButton* defaultButton = new BButton(frame, "ok button", "Ok",
											 new BMessage(MSG_DONE),
											 B_FOLLOW_RIGHT | B_FOLLOW_TOP);
		defaultButton->ResizeToPreferred();
		BButton* cancelButton = new BButton(frame, "cancel button", "Cancel",
											new BMessage(MSG_CANCEL),
											B_FOLLOW_RIGHT | B_FOLLOW_TOP);
		cancelButton->ResizeToPreferred();

		frame.bottom = frame.top + (defaultButton->Frame().Height() + 16);
		frame.right = frame.left + fPanel->fColorPickerView->Frame().Width();
		BBox* buttonBox = new BBox(frame, "button group",
			B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM,
			B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP, B_PLAIN_BORDER);

		fPanel->ResizeTo(frame.Width(),
			fPanel->fColorPickerView->Frame().Height() + frame.Height() + 1);

		frame = fPanel->Bounds();
		BView* topView = new BView(frame, "bg", B_FOLLOW_ALL, 0);
		topView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

		buttonBox->MoveTo(frame.left,
			frame.bottom - buttonBox->Frame().Height());

		defaultButton->MoveTo(frame.right - defaultButton->Frame().Width() - 10,
							  frame.top + 8);
		buttonBox->AddChild(defaultButton);

		cancelButton->MoveTo(defaultButton->Frame().left - 10
								- cancelButton->Frame().Width(),
							 frame.top + 8);
		buttonBox->AddChild(cancelButton);

		topView->AddChild(fPanel->fColorPickerView);
		topView->AddChild(buttonBox);

		fPanel->SetDefaultButton(defaultButton);

		if (fPanel->fWindow)
			fPanel->AddToSubset(fPanel->fWindow);
		else
			fPanel->SetFeel(B_FLOATING_APP_WINDOW_FEEL);

		fPanel->AddChild(topView);
	}

private:
	ColorPickerPanel*				fPanel;
};


#endif // COLOR_PICKER_PANEL_PLATFORM_DELEGATE_H
