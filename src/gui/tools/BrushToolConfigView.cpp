/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "BrushToolConfigView.h"

#include <stdio.h>

#include <CheckBox.h>
#include <GroupLayoutBuilder.h>
#include <SeparatorView.h>

#include "BrushTool.h"
#include "DualSlider.h"
#include "Tool.h"

enum {
	MSG_OPACITY_VALUE		= 'opvl',
	MSG_OPACITY_CONTROL		= 'opcn',

	MSG_RADIUS_VALUE		= 'rdvl',
	MSG_RADIUS_CONTROL		= 'rdcn',

	MSG_HARDNESS_VALUE		= 'hdvl',
	MSG_HARDNESS_CONTROL	= 'hdcn',

	MSG_SPACING_VALUE		= 'spvl',

	MSG_SUBPIXELS			= 'sbpx',
	MSG_SOLID				= 'slid',
	MSG_TILT				= 'tilt',
};

// constructor
BrushToolConfigView::BrushToolConfigView(::Tool* tool)
	: ToolConfigView(tool)
{
	BGroupLayout* layout = new BGroupLayout(B_HORIZONTAL);
	SetLayout(layout);

	fOpacity = new DualSlider("opacity", "Opacity",
		new BMessage(MSG_OPACITY_VALUE),
		new BMessage(MSG_OPACITY_CONTROL), this);

	fRadius = new DualSlider("radius", "Radius",
		new BMessage(MSG_RADIUS_VALUE),
		new BMessage(MSG_RADIUS_CONTROL), this, 0.0f, 1.0f / 100.0f);

	fHardness = new DualSlider("hardness", "Hardness",
		new BMessage(MSG_HARDNESS_VALUE),
		new BMessage(MSG_HARDNESS_CONTROL), this);
	fHardness->SetMinEnabled(false);

	fSpacing = new DualSlider("spacing", "Spacing",
		new BMessage(MSG_SPACING_VALUE), NULL, this, 0.0f, 0.2f);
	fSpacing->SetMinEnabled(false);

	fSubpixels = new BCheckBox("subpixels", "Subpixels",
		new BMessage(MSG_SUBPIXELS));
	fSolid = new BCheckBox("solid", "Solid", new BMessage(MSG_SOLID));
	fTilt = new BCheckBox("tilt", "Tilt", new BMessage(MSG_TILT));

	fSubpixels->SetValue(B_CONTROL_ON);
	fTilt->SetValue(B_CONTROL_ON);

	BGroupLayoutBuilder(layout)
		.Add(fOpacity)
		.Add(fRadius)
		.Add(fHardness)
		.Add(fSpacing)
		.Add(new BSeparatorView(B_VERTICAL, B_PLAIN_BORDER))
		.AddGroup(B_VERTICAL, 0, 0.0f)
			.AddGroup(B_HORIZONTAL, 0)
				.Add(fSolid)
				.Add(fTilt)
			.End()
			.Add(fSubpixels)
		.End()
		.SetInsets(5, 5, 5, 5)
	;
}

// destructor
BrushToolConfigView::~BrushToolConfigView()
{
}

// #pragma mark -

// AttachedToWindow
void
BrushToolConfigView::AttachedToWindow()
{
	fSubpixels->SetTarget(this);
	fSolid->SetTarget(this);
	fTilt->SetTarget(this);
}

// MessageReceived
void
BrushToolConfigView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_OPACITY_VALUE:
			_SetMinMaxOption(message, BrushTool::OPACITY_MIN,
				BrushTool::OPACITY_MAX);
			break;

		case MSG_OPACITY_CONTROL:
			Tool()->SetOption(BrushTool::OPACITY_CONTROLLED,
				fOpacity->IsMinEnabled());
			break;

		case MSG_RADIUS_VALUE:
			_SetMinMaxOption(message, BrushTool::RADIUS_MIN,
				BrushTool::RADIUS_MAX);
			break;

		case MSG_RADIUS_CONTROL:
			Tool()->SetOption(BrushTool::RADIUS_CONTROLLED,
				fRadius->IsMinEnabled());
			break;

		case MSG_HARDNESS_VALUE:
			_SetMinMaxOption(message, BrushTool::HARDNESS_MIN,
				BrushTool::HARDNESS_MAX);
			break;

		case MSG_HARDNESS_CONTROL:
			Tool()->SetOption(BrushTool::HARDNESS_CONTROLLED,
				fRadius->IsMinEnabled());
			break;

		case MSG_SPACING_VALUE:
			_SetMinMaxOption(message, BrushTool::SPACING,
				BrushTool::SPACING);
			break;

		case MSG_SUBPIXELS:
			Tool()->SetOption(BrushTool::SUBPIXELS,
				fSolid->Value() == B_CONTROL_ON);
			break;
		case MSG_SOLID:
			Tool()->SetOption(BrushTool::SOLID,
				fSolid->Value() == B_CONTROL_ON);
			break;
		case MSG_TILT:
			Tool()->SetOption(BrushTool::TILT_CONTROLLED,
				fTilt->Value() == B_CONTROL_ON);
			break;

		default:
			ToolConfigView::MessageReceived(message);
			break;
	}
}

// #pragma mark -

// UpdateStrings
void
BrushToolConfigView::UpdateStrings()
{
}

// SetActive
void
BrushToolConfigView::SetActive(bool active)
{
}

// SetEnabled
void
BrushToolConfigView::SetEnabled(bool enable)
{
	fOpacity->SetEnabled(enable);
	fRadius->SetEnabled(enable);
	fHardness->SetEnabled(enable);
	fSpacing->SetEnabled(enable);
	fSubpixels->SetEnabled(enable);
	fSolid->SetEnabled(enable);
	fTilt->SetEnabled(enable);
}

// #pragma mark -

// _SetMinMaxOption
void
BrushToolConfigView::_SetMinMaxOption(const BMessage* message,
	uint32 minOption, uint32 maxOption)
{
	float min;
	float max;
	if (message->FindFloat("min value", &min) == B_OK
		&& message->FindFloat("max value", &max) == B_OK) {
		if (minOption != maxOption)
			Tool()->SetOption(minOption, min);
		Tool()->SetOption(maxOption, max);
	}
}

