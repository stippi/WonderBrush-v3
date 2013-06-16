/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "RectangleToolConfigView.h"

#include <stdio.h>

#include <GroupLayoutBuilder.h>
#include <SeparatorView.h>
#include <Slider.h>
#include <TextControl.h>

#include "RectangleTool.h"
#include "RectangleToolState.h"
#include "support_ui.h"

enum {
	MSG_CORNER_RADIUS_SLIDER			= 'crds',
	MSG_CORNER_RADIUS_TEXT				= 'crdt',
};

// constructor
RectangleToolConfigView::RectangleToolConfigView(::Tool* tool)
	: ToolConfigView(tool)
{
	BGroupLayout* layout = new BGroupLayout(B_HORIZONTAL);
	SetLayout(layout);

	fRoundCornerRadiusSlider = new BSlider("corner radius slider",
		"Corner Radius", NULL, 1, 1024, B_HORIZONTAL, B_TRIANGLE_THUMB);
	fRoundCornerRadiusSlider->SetExplicitMinSize(BSize(80, B_SIZE_UNSET));
	fRoundCornerRadiusSlider->SetModificationMessage(
		new BMessage(MSG_CORNER_RADIUS_SLIDER));

	fRoundCornerRadiusTextControl = new BTextControl("corner radius text",
		"", "", new BMessage(MSG_CORNER_RADIUS_TEXT));

	BGroupLayoutBuilder(layout)
		.AddGroup(B_HORIZONTAL, 0.2f)
			.Add(fRoundCornerRadiusSlider, 0.8f)
			.Add(fRoundCornerRadiusTextControl, 0.2f)
		.End()

		.AddGlue()
		.SetInsets(5, 5, 5, 5)
	;
}

// destructor
RectangleToolConfigView::~RectangleToolConfigView()
{
}

// #pragma mark -

// AttachedToWindow
void
RectangleToolConfigView::AttachedToWindow()
{
	fRoundCornerRadiusSlider->SetTarget(this);
	fRoundCornerRadiusTextControl->SetTarget(this);
}

// DetachedFromWindow
void
RectangleToolConfigView::DetachedFromWindow()
{
}

// MessageReceived
void
RectangleToolConfigView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_CORNER_RADIUS_SLIDER:
		{
			float size = (float)_FromLinearSize(
				fRoundCornerRadiusSlider->Value());
			fTool->SetOption(RectangleTool::CORNER_RADIUS, size);
			set_text_control_float_value(fRoundCornerRadiusTextControl, size);
			break;
		}
		case MSG_CORNER_RADIUS_TEXT:
		{
			float size = get_text_control_float_value(
				fRoundCornerRadiusTextControl);
			fTool->SetOption(RectangleTool::CORNER_RADIUS, size);
			fRoundCornerRadiusSlider->SetValue(_ToLinearSize(size));
			break;
		}

		default:
			ToolConfigView::MessageReceived(message);
			break;
	}
}

// #pragma mark -

// UpdateStrings
void
RectangleToolConfigView::UpdateStrings()
{
}

// SetActive
void
RectangleToolConfigView::SetActive(bool active)
{
}

// SetEnabled
void
RectangleToolConfigView::SetEnabled(bool enable)
{
	fRoundCornerRadiusSlider->SetEnabled(enable);
	fRoundCornerRadiusTextControl->SetEnabled(enable);
}

// #pragma mark -

// _FromLinearSize
double
RectangleToolConfigView::_FromLinearSize(double value) const
{
	return from_linear(value, 1024.0);
}

// _ToLinearSize
double
RectangleToolConfigView::_ToLinearSize(double value) const
{
	return to_linear(value, 1024.0);
}
