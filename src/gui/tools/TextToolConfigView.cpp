/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "TextToolConfigView.h"

#include <stdio.h>

#include <CheckBox.h>
#include <GroupLayoutBuilder.h>
#include <ScrollView.h>
#include <SeparatorView.h>
#include <Slider.h>
#include <StringView.h>
#include <TextControl.h>
#include <TextView.h>

#include "TextTool.h"
#include "TextToolState.h"

enum {
	MSG_SIZE_SLIDER			= 'szsl',
	MSG_SIZE_TEXT			= 'sztx',
	MSG_SUBPIXELS			= 'sbpx',
	MSG_TEXT_CHANGED		= 'txch',
};

// NotifyingTextView
class NotifyingTextView : public BTextView {
public:
	NotifyingTextView(const char* name)
		: BTextView(name)
	{
	}

	void SetTarget(BHandler* target)
	{
		fMessenger = BMessenger(target);
	}

protected:
	virtual void InsertText(const char* inText, int32 inLength, int32 inOffset,
		const text_run_array* inRuns)
	{
		BTextView::InsertText(inText, inLength, inOffset, inRuns);
		fMessenger.SendMessage(MSG_TEXT_CHANGED);
	}
	
	virtual void DeleteText(int32 fromOffset, int32 toOffset)
	{
		BTextView::DeleteText(fromOffset, toOffset);
		fMessenger.SendMessage(MSG_TEXT_CHANGED);
	}

private:
	BMessenger	fMessenger;
};

// #pragma mark - TextToolConfigView

// constructor
TextToolConfigView::TextToolConfigView(::Tool* tool)
	: ToolConfigView(tool)
{
	BGroupLayout* layout = new BGroupLayout(B_HORIZONTAL);
	SetLayout(layout);

	fSizeLabel = new BStringView("", "Size");

	BAlignment labelAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE);
	fSizeLabel->SetExplicitAlignment(labelAlignment);

	fSizeSlider = new BSlider("size slider", "", new BMessage(MSG_SIZE_SLIDER),
		1, 1024, B_HORIZONTAL, B_TRIANGLE_THUMB);
	fSizeSlider->SetExplicitMinSize(BSize(80, B_SIZE_UNSET));
		
	fSizeTextControl = new BTextControl("size text y", "", "",
		new BMessage(MSG_SIZE_TEXT));

	fSubpixels = new BCheckBox("subpixels", "Subpixels",
		new BMessage(MSG_SUBPIXELS));

	fTextView = new NotifyingTextView("text view");

	BScrollView* scrollView = new BScrollView("text scroll view",
		fTextView, 0, false, true);

	BGroupLayoutBuilder(layout)
		.AddGroup(B_VERTICAL)
			.Add(fSizeLabel)
			.AddGroup(B_HORIZONTAL)
				.Add(fSizeSlider)
				.Add(fSizeTextControl)
			.End()
		.End()
		.Add(new BSeparatorView(B_VERTICAL, B_PLAIN_BORDER))
		.Add(fSubpixels)
		.Add(scrollView)
//		.AddGlue()
		.SetInsets(5, 5, 5, 5)
	;
}

// destructor
TextToolConfigView::~TextToolConfigView()
{
}

// #pragma mark -

// AttachedToWindow
void
TextToolConfigView::AttachedToWindow()
{
	fSizeSlider->SetTarget(this);
	fSizeTextControl->SetTarget(this);
	fSubpixels->SetTarget(this);
	fTextView->SetTarget(this);
}

// MessageReceived
void
TextToolConfigView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_SIZE_SLIDER:
			fTool->SetOption(TextTool::SIZE, fSizeSlider->Value());
			break;

		case MSG_SIZE_TEXT:
			fTool->SetOption(TextTool::SIZE, _Value(fSizeTextControl));
			break;

		case MSG_SUBPIXELS:
			fTool->SetOption(TextTool::SUBPIXELS,
				fSubpixels->Value() == B_CONTROL_ON);
			break;

		case MSG_TEXT_CHANGED:
			fTool->SetOption(TextTool::TEXT, fTextView->Text());
			break;

		case MSG_LAYOUT_CHANGED:
		{
			float size;
			if (message->FindFloat("size", &size) == B_OK) {
				fSizeSlider->SetValue(size);
				_SetValue(fSizeTextControl, size);
			}

			const char* text;
			if (message->FindString("text", &text) == B_OK) {
				fTextView->SetText(text);
			}
			
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
TextToolConfigView::UpdateStrings()
{
}

// SetActive
void
TextToolConfigView::SetActive(bool active)
{
}

// SetEnabled
void
TextToolConfigView::SetEnabled(bool enable)
{
	fSizeSlider->SetEnabled(enable);
	fSizeTextControl->SetEnabled(enable);
}

// #pragma mark - private

// _SetText
void
TextToolConfigView::_SetValue(BTextControl* control, float value) const
{
	char text[64];
	snprintf(text, sizeof(text), "%.2f", value);
	int32 selectionStart;
	int32 selectionEnd;
	control->TextView()->GetSelection(&selectionStart, &selectionEnd);
	bool selectionEndIsTextEnd
		= selectionEnd == control->TextView()->TextLength();

	control->SetText(text);

	if (selectionEndIsTextEnd)
		selectionEnd = control->TextView()->TextLength();
	control->TextView()->Select(selectionStart, selectionEnd);
}

// _Value
float
TextToolConfigView::_Value(BTextControl* control) const
{
	return atof(control->Text());
}

