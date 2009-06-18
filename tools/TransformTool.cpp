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
TransformTool::MakeViewState(StateView* view, Document* document)
{
	// TODO: Remove test code...
	TransformToolState* state = new TransformToolState(view,
		BRect(150, 150, 280, 250));
	Transformable t;
	t.ScaleBy(BPoint(220, 200), 1.2, 1.5);
	t.RotateBy(BPoint(200, 200), 10);
	state->SetObjectToCanvasTransformation(t);
	return state;
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

