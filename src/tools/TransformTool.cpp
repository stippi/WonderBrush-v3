/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "TransformTool.h"

#include <stdio.h>

#include "IconButton.h"
#include "MoveIcon.h"
#include "TransformToolState.h"

// constructor
TransformTool::TransformTool()
	:
	Tool("Transform")
{
}

// destructor
TransformTool::~TransformTool()
{
}

// #pragma mark -

// SaveSettings
status_t
TransformTool::SaveSettings(BMessage* message)
{
	return Tool::SaveSettings(message);
}

// LoadSettings
status_t
TransformTool::LoadSettings(BMessage* message)
{
	return Tool::LoadSettings(message);
}

// #pragma mark -

// ShortHelpMessage
const char*
TransformTool::ShortHelpMessage()
{
	return "Transform one or more objects on the canvas.";
}

// #pragma mark -

// MakeViewState
ViewState*
TransformTool::MakeViewState(StateView* view, Document* document,
	Selection* selection)
{
	return new(std::nothrow) TransformToolState(view, BRect(0, 0, -1, -1),
		document, selection);
}

// MakeConfigView
ToolConfigView*
TransformTool::MakeConfigView()
{
	return NULL;
}

// MakeIcon
IconButton*
TransformTool::MakeIcon()
{
	IconButton* button = new IconButton("transform", 0);
	button->SetIcon(kMoveIconBits, kMoveIconWidth, kMoveIconHeight,
		kMoveIconFormat);
	return button;
}

