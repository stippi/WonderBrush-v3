/*
 * Copyright 20012, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "TextTool.h"

#include <stdio.h>

#include "IconButton.h"
#include "TextToolConfigView.h"
#include "TextToolState.h"

// constructor
TextTool::TextTool()
	: Tool("Text")
{
}

// destructor
TextTool::~TextTool()
{
}

// #pragma mark -

// SaveSettings
status_t
TextTool::SaveSettings(BMessage* message)
{
	return Tool::SaveSettings(message);
}

// LoadSettings
status_t
TextTool::LoadSettings(BMessage* message)
{
	return Tool::LoadSettings(message);
}

// #pragma mark -

// ShortHelpMessage
const char*
TextTool::ShortHelpMessage()
{
	return "Create and edit text on the canvas.";
}

// #pragma mark -

// MakeViewState
ViewState*
TextTool::MakeViewState(StateView* view, Document* document,
	Selection* selection, CurrentColor* color)
{
	return new(std::nothrow) TextToolState(view, document, selection,
		color, BMessenger(ConfigView()));
}

// MakeConfigView
ToolConfigView*
TextTool::MakeConfigView()
{
	return new(std::nothrow) TextToolConfigView(this);
}

// MakeIcon
IconButton*
TextTool::MakeIcon()
{
	IconButton* button = new IconButton("text", 0);
	button->SetIcon(503, 32);
	button->TrimIcon(BRect(0, 0, 21, 21));
	return button;
}

// #pragma mark -

// SetOption
void
TextTool::SetOption(uint32 option, bool value)
{
	switch (option) {
		case SUBPIXELS:
			// TODO
			break;
	}
}

// SetOption
void
TextTool::SetOption(uint32 option, float value)
{
	TextToolState* state = static_cast<TextToolState*>(fViewState);

	switch (option) {
		case SIZE:
			state->SetSize(value);
			break;
	}

}

// SetOption
void
TextTool::SetOption(uint32 option, int32 value)
{
}

// SetOption
void
TextTool::SetOption(uint32 option, const char* value)
{
}

// Insert
void
TextTool::Insert(int32 textOffset, const char* text)
{
	TextToolState* state = static_cast<TextToolState*>(fViewState);
	state->Insert(textOffset, text);
}

// Remove
void
TextTool::Remove(int32 textOffset, int32 length)
{
	TextToolState* state = static_cast<TextToolState*>(fViewState);
	state->Remove(textOffset, length);
}

// SetFont
void
TextTool::SetFont(const char* family, const char* style)
{
	TextToolState* state = static_cast<TextToolState*>(fViewState);
	state->SetFont(family, style);
}

// SetSize
void
TextTool::SetSize(float size)
{
	TextToolState* state = static_cast<TextToolState*>(fViewState);
	state->SetSize(size);
}

// SetAlignment
void
TextTool::SetAlignment(uint32 alignment)
{
	TextToolState* state = static_cast<TextToolState*>(fViewState);
	state->SetTextAlignment(alignment);
}

// SetGlyphSpacing
void
TextTool::SetGlyphSpacing(double spacing)
{
	TextToolState* state = static_cast<TextToolState*>(fViewState);
	state->SetGlyphSpacing(spacing);
}

// SetFauxWeight
void
TextTool::SetFauxWeight(double fauxWeight)
{
	TextToolState* state = static_cast<TextToolState*>(fViewState);
	state->SetFauxWeight(fauxWeight);
}

// SetFauxItalic
void
TextTool::SetFauxItalic(double fauxItalic)
{
	TextToolState* state = static_cast<TextToolState*>(fViewState);
	state->SetFauxItalic(fauxItalic);
}
