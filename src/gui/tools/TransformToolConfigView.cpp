/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "TransformToolConfigView.h"

#include <stdio.h>

#include <CheckBox.h>
#include <GroupLayoutBuilder.h>
#include <SeparatorView.h>
#include <StringView.h>
#include <TextControl.h>

#include "Tool.h"

enum {
	MSG_TRANSLATE_X			= 'trnx',
	MSG_TRANSLATE_Y			= 'trny',
	MSG_ROTATE				= 'rota',
	MSG_SCALE_X				= 'sclx',
	MSG_SCALE_Y				= 'scly',
	MSG_SUBPIXELS			= 'sbpx'
};

// constructor
TransformToolConfigView::TransformToolConfigView(::Tool* tool)
	: ToolConfigView(tool)
{
	BGroupLayout* layout = new BGroupLayout(B_HORIZONTAL);
	SetLayout(layout);

	fTranslateLabel = new BStringView("", "Translate");
	fRotateLabel = new BStringView("", "Rotate");
	fScaleLabel = new BStringView("", "Scale");

	BAlignment labelAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE);
	fTranslateLabel->SetExplicitAlignment(labelAlignment);
	fRotateLabel->SetExplicitAlignment(labelAlignment);
	fScaleLabel->SetExplicitAlignment(labelAlignment);

	fTranslationX = new BTextControl("translate x", "X", "",
		new BMessage(MSG_TRANSLATE_X));
	fTranslationY = new BTextControl("translate y", "Y", "",
		new BMessage(MSG_TRANSLATE_Y));

	fRotate = new BTextControl("rotate", "Angle", "",
		new BMessage(MSG_ROTATE));

	fScaleX = new BTextControl("scale x", "X", "",
		new BMessage(MSG_SCALE_X));
	fScaleY = new BTextControl("scale y", "Y", "",
		new BMessage(MSG_SCALE_Y));

	fSubpixels = new BCheckBox("subpixels", "Subpixels",
		new BMessage(MSG_SUBPIXELS));

	BGroupLayoutBuilder(layout)
		.AddGroup(B_VERTICAL)
			.Add(fTranslateLabel)
			.AddGroup(B_HORIZONTAL)
				.Add(fTranslationX)
				.Add(fTranslationY)
			.End()
		.End()
		.Add(new BSeparatorView(B_VERTICAL, B_PLAIN_BORDER))
		.AddGroup(B_VERTICAL)
			.Add(fRotateLabel)
			.Add(fRotate)
		.End()
		.Add(new BSeparatorView(B_VERTICAL, B_PLAIN_BORDER))
		.AddGroup(B_VERTICAL)
			.Add(fScaleLabel)
			.AddGroup(B_HORIZONTAL)
				.Add(fScaleX)
				.Add(fScaleY)
			.End()
		.End()
		.Add(new BSeparatorView(B_VERTICAL, B_PLAIN_BORDER))
		.Add(fSubpixels)
		.AddGlue()
		.SetInsets(5, 5, 5, 5)
	;
}

// destructor
TransformToolConfigView::~TransformToolConfigView()
{
}

// #pragma mark -

// AttachedToWindow
void
TransformToolConfigView::AttachedToWindow()
{
	fTranslationX->SetTarget(this);
	fTranslationY->SetTarget(this);
	fRotate->SetTarget(this);
	fScaleX->SetTarget(this);
	fScaleY->SetTarget(this);
	fSubpixels->SetTarget(this);
}

// MessageReceived
void
TransformToolConfigView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_TRANSLATE_X:
			// TODO
			break;
		case MSG_TRANSLATE_Y:
			// TODO
			break;
		case MSG_ROTATE:
			// TODO
			break;
		case MSG_SCALE_X:
			// TODO
			break;
		case MSG_SCALE_Y:
			// TODO
			break;
		case MSG_SUBPIXELS:
			// TODO
			break;

		default:
			ToolConfigView::MessageReceived(message);
			break;
	}
}

// #pragma mark -

// UpdateStrings
void
TransformToolConfigView::UpdateStrings()
{
}

// SetActive
void
TransformToolConfigView::SetActive(bool active)
{
}

// SetEnabled
void
TransformToolConfigView::SetEnabled(bool enable)
{
	fTranslationX->SetEnabled(enable);
	fTranslationY->SetEnabled(enable);
	fScaleX->SetEnabled(enable);
	fScaleY->SetEnabled(enable);
}
