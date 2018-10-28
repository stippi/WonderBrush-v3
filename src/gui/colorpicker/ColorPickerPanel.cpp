/*
 * Copyright 2002-2006, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 *
 */

#include "ColorPickerPanel.h"
#include "ColorPickerPanelPlatformDelegate.h"

#include <stdio.h>

#include <Autolock.h>
#include <Application.h>
#include <Locker.h>

#include "support_ui.h"

#include "ColorPickerView.h"


ColorPickerPanel*
ColorPickerPanel::sDefaultPanel = NULL;

// constructor
ColorPickerPanel::ColorPickerPanel(BRect frame, rgb_color color,
								   SelectedColorMode mode,
								   BWindow* window,
								   BMessage* message, BHandler* target)
	: Panel(frame, "Pick Color",
			B_FLOATING_WINDOW_LOOK, B_FLOATING_SUBSET_WINDOW_FEEL,
			B_ASYNCHRONOUS_CONTROLS
				| B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_NOT_CLOSABLE
				| B_AUTO_UPDATE_SIZE_LIMITS),
	  fPlatformDelegate(new PlatformDelegate(this)),
	  fWindow(window),
	  fMessage(message),
	  fTarget(target)
{
	SetTitle("Pick Color");

	fColorPickerView = new ColorPickerView("color picker", color, mode);

	fPlatformDelegate->Init();
}

// destructor
ColorPickerPanel::~ColorPickerPanel()
{
	// TODO: Race condition with DefaultPanel().
	if (this == sDefaultPanel)
		sDefaultPanel = NULL;

	delete fMessage;

	delete fPlatformDelegate;
}

// Cancel
void
ColorPickerPanel::Cancel()
{
	PostMessage(MSG_CANCEL);
}

// MessageReceived
void
ColorPickerPanel::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_CANCEL:
		case MSG_DONE: {
			BMessage msg('PSTE');
			BLooper* looper = fTarget ? fTarget->Looper() : be_app;
			if (fMessage)
				msg = *fMessage;
			if (message->what == MSG_DONE)
				store_color_in_message(&msg, fColorPickerView->Color());
			msg.AddRect("panel frame", Frame());
			msg.AddInt32("panel mode", fColorPickerView->Mode());
			msg.AddBool("begin", true);
			looper->PostMessage(&msg, fTarget);
			PostMessage(B_QUIT_REQUESTED);
			break;
		}
		default:
			Panel::MessageReceived(message);
			break;
	}
}

// #pragma mark -

// SetColor
void
ColorPickerPanel::SetColor(rgb_color color)
{
	fColorPickerView->SetColor(color);
}

// SetMessage
void
ColorPickerPanel::SetMessage(BMessage* message)
{
	if (message != fMessage) {
		delete fMessage;
		fMessage = message;
	}
}

// SetTarget
void
ColorPickerPanel::SetTarget(BHandler* target)
{
	fTarget = target;
}

// DefaultPanel
ColorPickerPanel*
ColorPickerPanel::DefaultPanel()
{
	static BLocker locker;
	BAutolock _(&locker);

	if (sDefaultPanel == NULL) {
		sDefaultPanel = new ColorPickerPanel(BRect(0, 0, 350, 350),
			(rgb_color){ 0, 0, 0, 255 });
		sDefaultPanel->CenterOnScreen();
		sDefaultPanel->Show();
	} else {
		sDefaultPanel->Activate();
	}

	return sDefaultPanel;
}

