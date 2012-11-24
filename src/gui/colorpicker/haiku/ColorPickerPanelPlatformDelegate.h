/*
 * Copyright 2002-2006, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */
#ifndef COLOR_PICKER_PANEL_PLATFORM_DELEGATE_H
#define COLOR_PICKER_PANEL_PLATFORM_DELEGATE_H


#include <Box.h>
#include <Button.h>
#include <ControlLook.h>
#include <GroupView.h>
#include <LayoutBuilder.h>
#include <SeparatorView.h>

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
		fPanel->SetLayout(new BGroupLayout(B_VERTICAL));
		
		BButton* defaultButton = new BButton("ok button", "Ok",
			new BMessage(MSG_DONE));
		BButton* cancelButton = new BButton("cancel button", "Cancel",
			new BMessage(MSG_CANCEL));

		BView* topView = new BGroupView(B_VERTICAL);

		const float inset = be_control_look->DefaultLabelSpacing();

		BLayoutBuilder::Group<>(topView, B_VERTICAL, 0.0f)
			.Add(fPanel->fColorPickerView)
			.Add(new BSeparatorView(B_HORIZONTAL))
			.AddGroup(B_HORIZONTAL)
				.AddGlue()
				.Add(cancelButton)
				.Add(defaultButton)
				.SetInsets(inset, inset, inset, inset)
			.End()
		;

		fPanel->SetDefaultButton(defaultButton);

		if (fPanel->fWindow != NULL)
			fPanel->AddToSubset(fPanel->fWindow);
		else
			fPanel->SetFeel(B_FLOATING_APP_WINDOW_FEEL);

		fPanel->AddChild(topView);
	}

private:
	ColorPickerPanel*				fPanel;
};


#endif // COLOR_PICKER_PANEL_PLATFORM_DELEGATE_H
