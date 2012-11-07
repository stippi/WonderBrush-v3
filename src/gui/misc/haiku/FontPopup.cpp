/*
 * Copyright 2001-2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include <stdio.h>
#include <string.h>

#include <Font.h>
#include <Looper.h>
#include <Menu.h>
#include <PopUpMenu.h>
#include <String.h>

#include "FontPopup.h"

enum {
	MSG_SET_LABEL	= 'stlb',
};

class FontPopUpMenu : public BPopUpMenu {
public:
	FontPopUpMenu(const char *title, bool radioMode = true,
		bool autoRename = true, menu_layout layout = B_ITEMS_IN_COLUMN)
		: BPopUpMenu(title, radioMode, autoRename, layout)
	{
	}

	virtual	BPoint ScreenLocation()
	{
		BPoint p = BPopUpMenu::ScreenLocation();
		// just find the first marked item
		for (int32 i = 0; BMenuItem* item = ItemAt(i); i++) {
			if (!item->IsMarked())
				p.y -= item->Frame().Height();
			else
				break;
		}
		return p;
	}
};

// constructor
FontMenuItem::FontMenuItem(const char *label, font_family fontFamily,
	font_style fontStyle, BMessage* message)
	: BMenuItem(label, message)
	, fFont(new BFont(*be_plain_font))
{
	fFont->SetFamilyAndStyle(fontFamily, fontStyle);
}

// destructor
FontMenuItem::~FontMenuItem()
{
	delete fFont;
}

// GetContentSize
void
FontMenuItem::GetContentSize(float* width, float* height)
{
	if (fFont != NULL) {
		*width = ceilf(fFont->StringWidth(Label()) * 1.1);
		font_height fh;
		fFont->GetHeight(&fh);
		*height = ceilf((fh.ascent + fh.descent) * 1.1);
	} else
		_inherited::GetContentSize(width, height);
}

// DrawContent
void
FontMenuItem::DrawContent()
{
	if (fFont != NULL) {
		BPoint drawPoint(ContentLocation());
		Menu()->SetFont(fFont);
		font_height fh;
		fFont->GetHeight(&fh);
		drawPoint.y = (Frame().top + Frame().bottom) / 2.0
			- (fh.ascent + fh.descent) / 2.0 + fh.ascent;
		Menu()->SetDrawingMode(B_OP_OVER);
		Menu()->DrawString(Label(), drawPoint);
	} else
		_inherited::DrawContent();
}

// Invoke
status_t
FontMenuItem::Invoke(BMessage* message)
{
	BMessage* invokeMessage = message;
	if (invokeMessage == NULL)
		invokeMessage = Message();
	if (invokeMessage != NULL) {
		BMenu* menu = Menu();
		BMenu* super = menu->Supermenu();
		while (super != NULL) {
			super = menu->Supermenu();
			if (super != NULL)
				menu = super;
		}
		BView* parent = menu->Parent();
		if (parent != NULL) {
			BLooper* looper = parent->Looper();
			if (looper != NULL) {
				BMessage updateMessage(*invokeMessage);
				updateMessage.what = MSG_SET_LABEL;
				looper->PostMessage(&updateMessage, parent);
			}
		}
	}
	return _inherited::Invoke(message);
}

// GetFamilyAndStyle
void
FontMenuItem::GetFamilyAndStyle(font_family* family,
								font_style* style) const
{
	if (fFont != NULL)
		fFont->GetFamilyAndStyle(family, style);
}

// #pragma mark - FontPopup

// constructor
FontPopup::FontPopup(const char* label, bool subMenus, bool asLabel)
	: LabelPopup(label, new FontPopUpMenu("scanning"B_UTF8_ELLIPSIS),
		asLabel)
	, fSubMenus(subMenus)
{
	// we control the label and radio feature ourselves
	Menu()->SetLabelFromMarked(false);
	Menu()->SetRadioMode(false);
}

// MessageReceived
void
FontPopup::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_SET_LABEL: {
			const char* family;
			const char* style;
			if (message->FindString("font family", &family) == B_OK
				&& message->FindString("font style", &style) == B_OK) {
				SetFamilyAndStyle(family, style);
			}
			break;
		}
		default:
			LabelPopup::MessageReceived(message);
	}
}

// SetFamilyAndStyle
void
FontPopup::SetFamilyAndStyle(const char* family, const char* style)
{
	if (family == NULL || style == NULL)
		return;
	if (BMenuItem* superItem = Menu()->Superitem()) {
		BString label(family);
		label << ", " << style;
		superItem->SetLabel(label.String());
	}

	if (fSubMenus) {
		for (int32 i = 0; BMenuItem* item = Menu()->ItemAt(i); i++) {
			bool markStyle = false;
			if (strcmp(item->Label(), family) == 0) {
				item->SetMarked(true);
				markStyle = true;
			} else {
				item->SetMarked(false);
			}

			BMenu* styleMenu = item->Submenu();
			if (styleMenu == NULL)
				continue;

			for (int32 i = 0; i < styleMenu->CountItems(); i++) {
				BMenuItem* styleItem = styleMenu->ItemAt(i);
				if (markStyle && strcmp(styleItem->Label(), style) == 0)
					styleItem->SetMarked(true);
				else
					styleItem->SetMarked(false);
			}
		}
	} else {
		BString label(family);
		label << ", " << style;
		for (int32 i = 0; i < Menu()->CountItems(); i++) {
			BMenuItem* item = Menu()->ItemAt(i);
			if (strcmp(item->Label(), label.String()) == 0) {
				item->SetMarked(true);
			} else {
				item->SetMarked(false);
			}
		}
	}
}


