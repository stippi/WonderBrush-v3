/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "PickTool.h"

#include <stdio.h>

#include "IconButton.h"
#include "Document.h"
#include "PickIcon.h"
#include "PickToolState.h"

// constructor
PickTool::PickTool()
	:
	Tool("Pick")
{
}

// destructor
PickTool::~PickTool()
{
}

// #pragma mark -

// SaveSettings
status_t
PickTool::SaveSettings(BMessage* message)
{
	return Tool::SaveSettings(message);
}

// LoadSettings
status_t
PickTool::LoadSettings(BMessage* message)
{
	return Tool::LoadSettings(message);
}

// #pragma mark -

// ShortHelpMessage
const char*
PickTool::ShortHelpMessage()
{
	return "Pick objects by clicking on them on the Canvas.";
}

// #pragma mark -

// MakeViewState
ViewState*
PickTool::MakeViewState(StateView* view, Document* document,
	Selection* selection, CurrentColor* color)
{
	PickToolState* state = new PickToolState(view, document->RootLayer(),
		document, selection);
	return state;
}

// MakeConfigView
ToolConfigView*
PickTool::MakeConfigView()
{
	return NULL;
}

// MakeIcon
IconButton*
PickTool::MakeIcon()
{
	IconButton* button = new IconButton("pick", 0);
	button->SetIcon(kPickIconBits, kPickIconWidth, kPickIconHeight,
		kPickIconFormat);
	return button;
}

