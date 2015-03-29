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
	MSG_CANCEL					= 'cncl',
	MSG_LOCK_ASPECT				= 'lasp',
	MSG_SIZE_SELECTED			= 'szes',
	MSG_WIDTH_CHANGED			= 'wchg',
	MSG_HEIGHT_CHANGED			= 'hchg',
};


ResizeImagePanel::ResizeImagePanel(BWindow* parent, BRect frame)
	:
	BWindow(frame, B_TRANSLATE("Resize image"),
		B_FLOATING_WINDOW_LOOK, B_FLOATING_SUBSET_WINDOW_FEEL,
		B_ASYNCHRONOUS_CONTROLS | B_AUTO_UPDATE_SIZE_LIMITS
			| B_NOT_RESIZABLE | B_NOT_ZOOMABLE),
	fWidth(0),
	fHeight(0)
{
	AddToSubset(parent);

	fWidthField = new BTextControl(B_TRANSLATE("Width:"), "",
		new BMessage(MSG_WIDTH_CHANGED));
	_PrepareNumberTextControl(fWidthField);

	fHeightField = new BTextControl(B_TRANSLATE("Height:"), "",
		new BMessage(MSG_HEIGHT_CHANGED));
	_PrepareNumberTextControl(fHeightField);

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
		new BMessage(MSG_CANCEL));

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

	fWidthField->MakeFocus();
}


ResizeImagePanel::~ResizeImagePanel()
{
}


void
ResizeImagePanel::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_RESIZE:
			_Quit(true);
			break;
			
		case MSG_CANCEL:
			_Quit(false);
			break;

		case MSG_LOCK_ASPECT:
			break;

		case MSG_SIZE_SELECTED:
			// TODO: ...
			break;

		case MSG_WIDTH_CHANGED:
			_AdoptControlsToValues(true);
			_AdoptValuesToControls();
			break;

		case MSG_HEIGHT_CHANGED:
			_AdoptControlsToValues(false);
			_AdoptValuesToControls();
			break;

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


void
ResizeImagePanel::SetMessage(
	const BMessenger& messenger, const BMessage& message)
{
	fTarget = messenger;
	fMessage = message;
}


void
ResizeImagePanel::SetDocument(const DocumentRef& document)
{
	fDocument = document;

	fWidth = 0;
	fHeight = 0;
	if (fDocument.Get() != NULL) {
		BRect bounds = fDocument->Bounds();
		fWidth = bounds.IntegerWidth() + 1;
		fHeight = bounds.IntegerHeight() + 1;
	}

	_AdoptValuesToControls();
}


// #pragma mark - private


void
ResizeImagePanel::_FillCommonSizes(BMenu* menu) const
{
//	for (int i = 0; i < sizes.CountItems(); i++) {
//		const Size& size = sizes.ItemAtFast(i);
//		BMessage* message = new BMessage(MSG_SIZE_SELECTED);
//		message->AddInt32("width", size.width);
//		message->AddInt32("height", size.height);
//		BMenuItem* item = new BMenuItem(size.label, message);
//		menu->AddItem(item);
//	}
}


void
ResizeImagePanel::_PrepareNumberTextControl(BTextControl* control)
{
	// TODO: Accept only digits as input
}


void
ResizeImagePanel::_AdoptValuesToControls()
{
	BString string;
	string << fWidth;
	
	if (string != fWidthField->Text())
		fWidthField->SetText(string.String());

	string = "";
	string << fHeight;

	if (string != fHeightField->Text())
		fHeightField->SetText(string.String());
}


void
ResizeImagePanel::_AdoptControlsToValues(bool widthChanged)
{
	fWidth = atoi(fWidthField->Text());
	if (fWidth < 1)
		fWidth = 1;
	fHeight = atoi(fHeightField->Text());
	if (fHeight < 1)
		fHeight = 1;

	_LockAspectRatio(!widthChanged);
}


void
ResizeImagePanel::_LockAspectRatio(bool adjustWidth)
{
	if (fLockAspectField->Value() != B_CONTROL_ON)
		return;

	// TODO: ...
}


void
ResizeImagePanel::_Quit(bool success)
{
	if (success) {
		_AdoptControlsToValues(fWidthField->TextView()->IsFocus());
		
		fMessage.AddInt32("width", fWidth);
		fMessage.AddInt32("height", fHeight);
	}

	fMessage.AddRect("panel frame", Frame());

	fTarget.SendMessage(&fMessage);

	PostMessage(B_QUIT_REQUESTED, this);
}


