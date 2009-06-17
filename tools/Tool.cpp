/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "Tool.h"

#include <stdio.h>

#include "ViewState.h"

// constructor
Tool::Tool(const char* name)
	:
	BHandler(name),
	fConfigView(NULL),
	fIcon(NULL)
{
}

// destructor
Tool::~Tool()
{
	// NOTE: the GUI stuff is deleted by the window
	// to which it has been attached
}

// #pragma mark -

// SaveSettings
status_t
Tool::SaveSettings(BMessage* message)
{
	if (message != NULL)
		return B_OK;
	return B_BAD_VALUE;
}

// LoadSettings
status_t
Tool::LoadSettings(BMessage* message)
{
	if (message != NULL)
		return B_OK;
	return B_BAD_VALUE;
}

// #pragma mark -

// ConfigView
ToolConfigView*
Tool::ConfigView()
{
	if (fConfigView == NULL)
		fConfigView = MakeConfigView();
	return fConfigView;
}

// Icon
IconButton*
Tool::Icon()
{
	if (fIcon == NULL)
		fIcon = MakeIcon();
	return fIcon;
}

// ShortHelpMessage
const char*
Tool::ShortHelpMessage()
{
	return NULL;
}

// #pragma mark -

// Confirm
status_t
Tool::Confirm()
{
	return B_OK;
}

// Cancel
status_t
Tool::Cancel()
{
	return B_OK;
}


