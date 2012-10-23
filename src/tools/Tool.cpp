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
	fViewState(NULL),
	fConfigView(NULL),
	fIcon(NULL)
{
}

// destructor
Tool::~Tool()
{
	delete fViewState;
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

// ToolViewState
ViewState*
Tool::ToolViewState(StateView* view, Document* document, Selection* selection)
{
	if (fViewState == NULL)
		fViewState = MakeViewState(view, document, selection);
	return fViewState;
}

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

// #pragma mark -

// SetOption
void
Tool::SetOption(uint32 option, bool value)
{
}

// SetOption
void
Tool::SetOption(uint32 option, float value)
{
}

// SetOption
void
Tool::SetOption(uint32 option, int32 value)
{
}

// SetOption
void
Tool::SetOption(uint32 option, const char* value)
{
}


