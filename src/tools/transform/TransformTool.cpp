/*
 * Copyright 2009-2010, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "TransformTool.h"

#include <stdio.h>

#include "IconButton.h"
#include "MoveIcon.h"
#include "TransformToolConfigView.h"
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
		document, selection, BMessenger(ConfigView()));
}

// MakeConfigView
ToolConfigView*
TransformTool::MakeConfigView()
{
	return new(std::nothrow) TransformToolConfigView(this);
}

// MakeIcon
IconButton*
TransformTool::MakeIcon()
{
	IconButton* button = new IconButton("transform", 0);
//	button->SetIcon(kMoveIconBits, kMoveIconWidth, kMoveIconHeight,
//		kMoveIconFormat);
	button->SetIcon(501, 32);
	button->TrimIcon(BRect(0, 0, 21, 21));
	return button;
}

// #pragma mark -

// SetOption
void
TransformTool::SetOption(uint32 option, bool value)
{
	switch (option) {
		case SUBPIXELS:
			// TODO: ...
			break;
	}
}

// SetOption
void
TransformTool::SetOption(uint32 option, float value)
{
	TransformToolState* state = static_cast<TransformToolState*>(fViewState);

	const BRect& box = state->Box();
	BRect modifiedBox = state->ModifiedBox();

	switch (option) {
		case TRANSLATION_X:
			modifiedBox.OffsetTo(box.left + value, modifiedBox.top);
			state->SetModifiedBox(modifiedBox);
			break;
		case TRANSLATION_Y:
			modifiedBox.OffsetTo(modifiedBox.left, box.top + value);
			state->SetModifiedBox(modifiedBox);
			break;

		case ROTATION:
			state->SetLocalRotation(value);
			break;

		case SCALE_X:
			modifiedBox.right = modifiedBox.left + box.Width() * value;
			state->SetModifiedBox(modifiedBox);
			break;
		case SCALE_Y:
			modifiedBox.bottom = modifiedBox.top + box.Height() * value;
			state->SetModifiedBox(modifiedBox);
			break;
	}
}

// SetOption
void
TransformTool::SetOption(uint32 option, int32 value)
{
}

