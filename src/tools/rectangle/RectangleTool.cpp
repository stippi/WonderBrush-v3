/*
 * Copyright 2013-2020, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "RectangleTool.h"

#include <stdio.h>

#include "IconButton.h"
#include "RectangleToolConfigView.h"
#include "RectangleToolState.h"

// constructor
RectangleTool::RectangleTool()
	: Tool("Path")
{
}

// destructor
RectangleTool::~RectangleTool()
{
}

// #pragma mark -

// SaveSettings
status_t
RectangleTool::SaveSettings(BMessage* message)
{
	return Tool::SaveSettings(message);
}

// LoadSettings
status_t
RectangleTool::LoadSettings(BMessage* message)
{
	return Tool::LoadSettings(message);
}

// #pragma mark -

// ShortHelpMessage
const char*
RectangleTool::ShortHelpMessage()
{
	return "Create and edit rectangles on the canvas.";
}

// #pragma mark -

// Confirm
status_t
RectangleTool::Confirm()
{
	((RectangleToolState*)ToolViewState())->Confirm();
	return B_OK;
}

// Cancel
status_t
RectangleTool::Cancel()
{
	((RectangleToolState*)ToolViewState())->Cancel();
	return B_OK;
}

// #pragma mark -

// MakeViewState
ViewState*
RectangleTool::MakeViewState(StateView* view, Document* document,
	Selection* selection, CurrentColor* color)
{
	return new(std::nothrow) RectangleToolState(view, this, document, selection,
		color, BMessenger(ConfigView()));
}

// MakeConfigView
ToolConfigView*
RectangleTool::MakeConfigView()
{
	return new(std::nothrow) RectangleToolConfigView(this);
}

// MakeIcon
IconButton*
RectangleTool::MakeIcon()
{
	IconButton* button = new IconButton("rectangle", 0);
	button->SetIcon(505, IconSize());
	button->TrimIcon(IconTrimRect());
	return button;
}

// #pragma mark -

// SetOption
void
RectangleTool::SetOption(uint32 option, bool value)
{
}

// SetOption
void
RectangleTool::SetOption(uint32 option, float value)
{
	RectangleToolState* state = static_cast<RectangleToolState*>(fViewState);

	switch (option) {
		case CORNER_RADIUS:
			state->SetRoundCornerRadius(value);
			break;
	}
}

// SetOption
void
RectangleTool::SetOption(uint32 option, int32 value)
{
}

// SetOption
void
RectangleTool::SetOption(uint32 option, const char* value)
{
}

