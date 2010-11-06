/*
 * Copyright 2009-2010, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "BrushTool.h"

#include <stdio.h>

#include "BrushIcon.h"
#include "BrushToolState.h"
#include "IconButton.h"

// constructor
BrushTool::BrushTool()
	: Tool("Brush")
{
}

// destructor
BrushTool::~BrushTool()
{
}

// #pragma mark -

// SaveSettings
status_t
BrushTool::SaveSettings(BMessage* message)
{
	return Tool::SaveSettings(message);
}

// LoadSettings
status_t
BrushTool::LoadSettings(BMessage* message)
{
	return Tool::LoadSettings(message);
}

// #pragma mark -

// ShortHelpMessage
const char*
BrushTool::ShortHelpMessage()
{
	return "Paint brush strokes onto the canvas.";
}

// #pragma mark -

// MakeViewState
ViewState*
BrushTool::MakeViewState(StateView* view, Document* document,
	Selection* selection)
{
	return new(std::nothrow) BrushToolState(view, document, selection);
}

// MakeConfigView
ToolConfigView*
BrushTool::MakeConfigView()
{
	return NULL;
}

// MakeIcon
IconButton*
BrushTool::MakeIcon()
{
	IconButton* button = new IconButton("brush", 0);
//	button->SetIcon(kBrushIconBits, kBrushIconWidth, kBrushIconHeight,
//		kBrushIconFormat);
	button->SetIcon(502);
	button->TrimIcon(BRect(0, 0, 21, 21));
	return button;
}

