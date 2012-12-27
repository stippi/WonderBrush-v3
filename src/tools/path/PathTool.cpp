/*
 * Copyright 20012, Stephan Aßmus <superstippi@gmx.de>
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

// MakeViewState
ViewState*
PathTool::MakeViewState(StateView* view, Document* document,
	Selection* selection, CurrentColor* color)
{
	return new(std::nothrow) PathToolState(view, document, selection,
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
	button->SetIcon(504, 32);
	button->TrimIcon(BRect(0, 0, 21, 21));
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
