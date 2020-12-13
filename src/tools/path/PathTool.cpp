/*
 * Copyright 2012-2020, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "PathTool.h"

#include <stdio.h>

#include "IconButton.h"
#include "PathToolConfigView.h"
#include "PathToolState.h"

// constructor
PathTool::PathTool()
	: Tool("Path")
{
}

// destructor
PathTool::~PathTool()
{
}

// #pragma mark -

// SaveSettings
status_t
PathTool::SaveSettings(BMessage* message)
{
	return Tool::SaveSettings(message);
}

// LoadSettings
status_t
PathTool::LoadSettings(BMessage* message)
{
	return Tool::LoadSettings(message);
}

// #pragma mark -

// ShortHelpMessage
const char*
PathTool::ShortHelpMessage()
{
	return "Create and edit paths on the canvas.";
}

// #pragma mark -

// Confirm
status_t
PathTool::Confirm()
{
	((PathToolState*)ToolViewState())->Confirm();
	return B_OK;
}

// Cancel
status_t
PathTool::Cancel()
{
	((PathToolState*)ToolViewState())->Cancel();
	return B_OK;
}

// #pragma mark -

// MakeViewState
ViewState*
PathTool::MakeViewState(StateView* view, Document* document,
	Selection* selection, CurrentColor* color)
{
	return new(std::nothrow) PathToolState(view, this, document, selection,
		color, BMessenger(ConfigView()));
}

// MakeConfigView
ToolConfigView*
PathTool::MakeConfigView()
{
	return new(std::nothrow) PathToolConfigView(this);
}

// MakeIcon
IconButton*
PathTool::MakeIcon()
{
	IconButton* button = new IconButton("path", 0);
	button->SetIcon(504, IconSize());
	button->TrimIcon(IconTrimRect());
	return button;
}

// #pragma mark -

// SetOption
void
PathTool::SetOption(uint32 option, bool value)
{
}

// SetOption
void
PathTool::SetOption(uint32 option, float value)
{
}

// SetOption
void
PathTool::SetOption(uint32 option, int32 value)
{
}

// SetOption
void
PathTool::SetOption(uint32 option, const char* value)
{
}
