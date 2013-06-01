/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "RectangleToolConfigView.h"

#include <stdio.h>

#include <CheckBox.h>
#include <GroupLayoutBuilder.h>
#include <SeparatorView.h>

#include "PathTool.h"
#include "PathToolState.h"

enum {
	MSG_SUBPIXELS			= 'sbpx',
};

// constructor
RectangleToolConfigView::RectangleToolConfigView(::Tool* tool)
	: ToolConfigView(tool)
{
	BGroupLayout* layout = new BGroupLayout(B_HORIZONTAL);
	SetLayout(layout);

	fSubpixels = new BCheckBox("subpixels", "Subpixels",
		new BMessage(MSG_SUBPIXELS));

	BGroupLayoutBuilder(layout)
		.Add(fSubpixels)
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
	fSubpixels->SetTarget(this);
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
		case MSG_SUBPIXELS:
			break;

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
	fSubpixels->SetEnabled(enable);
}

