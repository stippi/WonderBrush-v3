/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "PathToolConfigView.h"

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
PathToolConfigView::PathToolConfigView(::Tool* tool)
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
PathToolConfigView::~PathToolConfigView()
{
}

// #pragma mark -

// AttachedToWindow
void
PathToolConfigView::AttachedToWindow()
{
	fSubpixels->SetTarget(this);
}

// DetachedFromWindow
void
PathToolConfigView::DetachedFromWindow()
{
}

// MessageReceived
void
PathToolConfigView::MessageReceived(BMessage* message)
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
PathToolConfigView::UpdateStrings()
{
}

// SetActive
void
PathToolConfigView::SetActive(bool active)
{
}

// SetEnabled
void
PathToolConfigView::SetEnabled(bool enable)
{
	fSubpixels->SetEnabled(enable);
}

