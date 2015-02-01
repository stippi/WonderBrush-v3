/*
 * Copyright 2015, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "ResizeImagePanel.h"

#include <algorithm>
#include <stdio.h>

#include <Autolock.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <MenuField.h>
#include <PopUpMenu.h>
#include <TextControl.h>

#include "List.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ResizeImagePanel"


enum {
	MSG_RESIZE					= 'rsze',
	MSG_LOCK_ASPECT				= 'lasp',
	MSG_SIZE_SELECTED			= 'szes',
};


//static void
//add_sizes_to_menu(const SizeList& sizes, BMenu* menu)
//{
//	for (int i = 0; i < sizes.CountItems(); i++) {
//		const Size& size = sizes.ItemAtFast(i);
//		BMessage* message = new BMessage(MSG_SIZE_SELECTED);
//		message->AddInt32("width", size.width);
//		message->AddInt32("height", size.height);
//		BMenuItem* item = new BMenuItem(size.label, message);
//		menu->AddItem(item);
//	}
//}


ResizeImagePanel::ResizeImagePanel(BWindow* parent, BRect frame)
	:
	BWindow(frame, B_TRANSLATE("Resize image"),
		B_FLOATING_WINDOW_LOOK, B_FLOATING_SUBSET_WINDOW_FEEL,
		B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS
			| B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
	AddToSubset(parent);

	fWidthField = new BTextControl(B_TRANSLATE("Width:"), "", NULL);
	fHeightField = new BTextControl(B_TRANSLATE("Height:"), "", NULL);

	fLockAspectField = new BCheckBox("lock aspect",
		B_TRANSLATE("Lock aspect ratio"), new BMessage(MSG_LOCK_ASPECT));

	BPopUpMenu* sizesMenu = new BPopUpMenu(B_TRANSLATE("<select>"));
	fCommonSizesField = new BMenuField("common sizes",
		B_TRANSLATE("Set size:"), sizesMenu);

//	add_sizes_to_menu(fModel.SupportedLanguages(), languagesMenu);
	sizesMenu->SetTargetForItems(this);

	fOKButton = new BButton("resize", B_TRANSLATE("Resize"),
		new BMessage(MSG_RESIZE));
	fCancelButton = new BButton("cancel", B_TRANSLATE("Cancel"),
		new BMessage(B_QUIT_REQUESTED));

	// Build layout
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.AddGrid()
			.AddTextControl(fWidthField, 0, 0)
			.AddTextControl(fHeightField, 0, 1)
			.Add(fLockAspectField, 0, 2, 2)
			.AddMenuField(fCommonSizesField, 0, 3)
			.AddGlue(0, 3)
		.End()
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(fCancelButton)
			.Add(fOKButton)
		.End()
		.SetInsets(B_USE_WINDOW_INSETS)
	;

	SetDefaultButton(fOKButton);

	CenterIn(parent->Frame());
}


ResizeImagePanel::~ResizeImagePanel()
{
}


void
ResizeImagePanel::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_RESIZE:
			break;

		case MSG_LOCK_ASPECT:
			break;

		case MSG_SIZE_SELECTED:
			break;

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


void
ResizeImagePanel::SetOnSuccessMessage(
	const BMessenger& messenger, const BMessage& message)
{
	fOnSuccessTarget = messenger;
	fOnSuccessMessage = message;
}
