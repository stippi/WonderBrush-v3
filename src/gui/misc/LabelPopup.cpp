/*
 * Copyright 2001-2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "LabelPopup.h"

#include <stdio.h>

#include <MenuBar.h>
#include <MenuItem.h>
#include <OS.h>
#include <PopUpMenu.h>

#define LABEL_DIST 8.0
#define DIVIDER_DIST 3.0

// constructor
LabelPopup::LabelPopup(const char* label, BMenu* menu, bool fixedSize,
		bool asLabel)
	:
	BMenuField(label, label, menu ? menu : new BPopUpMenu("popup"), 0)
{
	if (Menu()) {
		Menu()->SetRadioMode(true);
		MenuItem()->SetMarked(false);
	}
	const BFont* labelFont = asLabel ? be_bold_font : be_plain_font;
	SetFont(labelFont);

	if (fixedSize) {
		// TODO: Mess with explicite max size...
	}
}

// destructor
LabelPopup::~LabelPopup()
{
}

// RefreshItemLabel
void
LabelPopup::RefreshItemLabel()
{
	if (BMenuItem* item = Menu()->FindMarked())
		item->SetMarked(true);
}
