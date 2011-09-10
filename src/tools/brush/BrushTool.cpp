/*
 * Copyright 2009-2010, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "BrushTool.h"

#include <stdio.h>

#include "BrushIcon.h"
#include "BrushToolConfigView.h"
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
	return new(std::nothrow) BrushToolState(view, document, selection, fBrush);
}

// MakeConfigView
ToolConfigView*
BrushTool::MakeConfigView()
{
	return new(std::nothrow) BrushToolConfigView(this);
}

// MakeIcon
IconButton*
BrushTool::MakeIcon()
{
	IconButton* button = new IconButton("brush", 0);
//	button->SetIcon(kBrushIconBits, kBrushIconWidth, kBrushIconHeight,
//		kBrushIconFormat);
	button->SetIcon(502, 32);
	button->TrimIcon(BRect(0, 0, 21, 21));
	return button;
}

// #pragma mark -

// SetOption
void
BrushTool::SetOption(uint32 option, bool value)
{
	switch (option) {
		case OPACITY_CONTROLLED:
			fBrush.SetFlags(Brush::FLAG_PRESSURE_CONTROLS_APHLA, value);
			break;
		case RADIUS_CONTROLLED:
			fBrush.SetFlags(Brush::FLAG_PRESSURE_CONTROLS_RADIUS, value);
			break;
		case HARDNESS_CONTROLLED:
			fBrush.SetFlags(Brush::FLAG_PRESSURE_CONTROLS_HARDNESS, value);
			break;
		case SOLID:
			fBrush.SetFlags(Brush::FLAG_SOLID, value);
			break;
		case TILT_CONTROLLED:
			fBrush.SetFlags(Brush::FLAG_TILT_CONTROLS_SHAPE, value);
			break;
	}
}

// SetOption
void
BrushTool::SetOption(uint32 option, float value)
{
	switch (option) {
		case OPACITY_MIN:
			fBrush.SetMinOpacity(value);
			break;
		case OPACITY_MAX:
			fBrush.SetMaxOpacity(value);
			break;

		case RADIUS_MIN:
			fBrush.SetMinRadius(value * 100.0f);
			break;
		case RADIUS_MAX:
			fBrush.SetMaxRadius(value * 100.0f);
			break;

		case HARDNESS_MIN:
			fBrush.SetMinHardness(value);
			break;
		case HARDNESS_MAX:
			fBrush.SetMaxHardness(value);
			break;
	}
		
}

// SetOption
void
BrushTool::SetOption(uint32 option, int32 value)
{
}


