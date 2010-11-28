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

#include "TransformTool.h"
#include "TransformToolState.h"

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
			fTool->SetOption(TransformTool::TRANSLATION_X,
				_Value(fTranslationX));
			break;
		case MSG_TRANSLATE_Y:
			fTool->SetOption(TransformTool::TRANSLATION_Y,
				_Value(fTranslationY));
			break;
		case MSG_ROTATE:
			fTool->SetOption(TransformTool::ROTATION, _Value(fRotate));
			break;
		case MSG_SCALE_X:
			fTool->SetOption(TransformTool::SCALE_X, _Value(fScaleX));
			break;
		case MSG_SCALE_Y:
			fTool->SetOption(TransformTool::SCALE_Y, _Value(fScaleY));
			break;
		case MSG_SUBPIXELS:
			fTool->SetOption(TransformTool::SUBPIXELS,
				fSubpixels->Value() == B_CONTROL_ON);
			break;

		case MSG_TRANSFORMATION_CHANGED:
		{
			BPoint pivot;
			BPoint translation;
			double rotation;
			double scaleX;
			double scaleY;

			message->FindFloat("pivot x", &pivot.x);
			message->FindFloat("pivot y", &pivot.y);
			message->FindFloat("translation x", &translation.x);
			message->FindFloat("translation y", &translation.y);
			message->FindDouble("rotation", &rotation);
			message->FindDouble("scale x", &scaleX);
			message->FindDouble("scale y", &scaleY);

			_SetValue(fTranslationX, translation.x);
			_SetValue(fTranslationY, translation.y);
			_SetValue(fRotate, rotation);
			_SetValue(fScaleX, scaleX);
			_SetValue(fScaleY, scaleY);
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

// #pragma mark - private

// _SetText
void
TransformToolConfigView::_SetValue(BTextControl* control, float value) const
{
	char text[64];
	snprintf(text, sizeof(text), "%.2f", value);
	int32 selectionStart;
	int32 selectionEnd;
	control->TextView()->GetSelection(&selectionStart, &selectionEnd);
	control->SetText(text);
	control->TextView()->Select(selectionStart, selectionEnd);
}

// _Value
float
TransformToolConfigView::_Value(BTextControl* control) const
{
	return atof(control->Text());
}

