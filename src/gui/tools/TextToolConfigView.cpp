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

#include "FontPopup.h"
#include "FontRegistry.h"
#include "TextTool.h"
#include "TextToolState.h"

enum {
	MSG_FONT_SELECTED		= 'fnsl',
	MSG_SIZE_SLIDER			= 'szsl',
	MSG_SIZE_TEXT			= 'sztx',
	MSG_SUBPIXELS			= 'sbpx',

	MSG_INSERT				= 'isrt',
	MSG_REMOVE				= 'rmov',

	MSG_SELECTION_CHANGED	= 'slch',
	MSG_SHOW_TEXT_OFFSET	= 'shwo',
};

// NotifyingTextView
class NotifyingTextView : public BTextView {
public:
	NotifyingTextView(const char* name)
		: BTextView(name)
		, fNotificationsEnabled(true)
	{
	}

	void SetTarget(BHandler* target)
	{
		fMessenger = BMessenger(target);
	}

	virtual void Select(int32 startOffset, int32 endOffset)
	{
		BTextView::Select(startOffset, endOffset);
		_SelectionChanged(startOffset, endOffset);
	}

	virtual void ScrollToOffset(int32 inOffset)
	{
		BTextView::ScrollToOffset(inOffset);
		if (!fNotificationsEnabled)
			return;

		BMessage message(MSG_SHOW_TEXT_OFFSET);
		message.AddInt32("offset", inOffset);
		fMessenger.SendMessage(&message);
	}

	void SetNotificationsEnabled(bool enabled)
	{
		fNotificationsEnabled = enabled;
	}

protected:
	void _UpdateSelection()
	{
		int32 startOffset;
		int32 endOffset;
		GetSelection(&startOffset, &endOffset);
		_SelectionChanged(startOffset, endOffset);
	}

	void _SelectionChanged(int startOffset, int endOffset)
	{
		if (!fNotificationsEnabled)
			return;

		BMessage message(MSG_SELECTION_CHANGED);
		message.AddInt32("start offset", startOffset);
		message.AddInt32("end offset", endOffset);
		fMessenger.SendMessage(&message);
	}

	virtual void InsertText(const char* inText, int32 inLength, int32 inOffset,
		const text_run_array* inRuns)
	{
		BTextView::InsertText(inText, inLength, inOffset, inRuns);
		if (!fNotificationsEnabled)
			return;

		BMessage message(MSG_INSERT);
		message.AddString("text", inText);
		message.AddInt32("offset", inOffset);

		fMessenger.SendMessage(&message);

		_UpdateSelection();
	}

	virtual void DeleteText(int32 fromOffset, int32 toOffset)
	{
		BTextView::DeleteText(fromOffset, toOffset);
		if (!fNotificationsEnabled)
			return;

		BMessage message(MSG_REMOVE);
		message.AddInt32("offset", fromOffset);
		message.AddInt32("length", toOffset - fromOffset);

		fMessenger.SendMessage(&message);

		_UpdateSelection();
	}

private:
	BMessenger	fMessenger;
	bool		fNotificationsEnabled;
};

// #pragma mark - TextToolConfigView

// constructor
TextToolConfigView::TextToolConfigView(::Tool* tool)
	: ToolConfigView(tool)
{
	BGroupLayout* layout = new BGroupLayout(B_HORIZONTAL);
	SetLayout(layout);

	fFontPopup = new FontPopup("Font", true);

	fSizeSlider = new BSlider("size slider", "Size",
		new BMessage(MSG_SIZE_SLIDER),
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
			.Add(fFontPopup)
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

	FontRegistry* fontRegistry = FontRegistry::Default();
	if (fontRegistry->LockWithTimeout(3000) == B_OK) {
		_PolulateFontMenu(fFontPopup->Menu(), this, NULL, NULL);
		fontRegistry->Unlock();
	}
	fontRegistry->StartWatching(this, FONTS_CHANGED);
}

// MessageReceived
void
TextToolConfigView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_FONTS_CHANGED:
			printf("MSG_FONTS_CHANGED\n");
			break;

		case MSG_FONT_SELECTED:
		{
			const char* family;
			if (message->FindString("font family", &family) != B_OK)
				break;

			const char* style;
			if (message->FindString("font style", &style) != B_OK)
				break;

			FontRegistry* registry = FontRegistry::Default();
			if (registry->Lock()) {
				const char* fontFilePath = registry->FontFileFor(
					family,
					style
				);
				if (fontFilePath != NULL)
					((TextTool*)fTool)->SetFont(fontFilePath);
				registry->Unlock();
			}
			break;
		}

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

		case MSG_INSERT:
		{
			const char* text;
			if (message->FindString("text", &text) != B_OK)
				break;

			int32 textOffset;
			if (message->FindInt32("offset", &textOffset) != B_OK)
				break;

			((TextTool*)fTool)->Insert(textOffset, text);
			break;
		}

		case MSG_REMOVE:
		{
			int32 textOffset;
			if (message->FindInt32("offset", &textOffset) != B_OK)
				break;

			int32 length;
			if (message->FindInt32("length", &length) != B_OK)
				break;

			((TextTool*)fTool)->Remove(textOffset, length);
			break;
		}

		case MSG_SELECTION_CHANGED:
		{
			int32 startOffset;
			int32 endOffset;
			if (message->FindInt32("start offset", &startOffset) == B_OK
				&& message->FindInt32("end offset", &endOffset) == B_OK) {
				dynamic_cast<TextToolState*>(fTool->ToolViewState())
					->SelectionChanged(startOffset, endOffset);
			}
			break;
		}

		case MSG_SHOW_TEXT_OFFSET:
		{
			// TODO: Make the ViewState scroll the canvas to show the
			// text offset.
			break;
		}

		case MSG_LAYOUT_CHANGED:
		{
			float size;
			if (message->FindFloat("size", &size) == B_OK) {
				fSizeSlider->SetValue(size);
				_SetValue(fSizeTextControl, size);
			}

			const char* text;
			if (message->FindString("text", &text) == B_OK) {
				fTextView->SetNotificationsEnabled(false);
				fTextView->SetText(text);
				fTextView->SetNotificationsEnabled(true);
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

// _PolulateFontMenu
void
TextToolConfigView::_PolulateFontMenu(BMenu* menu, BHandler* target,
	const char* markedFamily, const char* markedStyle)
{
	if (menu == NULL)
		return;

	FontRegistry* manager = FontRegistry::Default();

	if (!manager->Lock())
		return;

	while (menu->CountItems() > 0) {
		delete menu->RemoveItem(0L);
	}

	BMenu* fontMenu = NULL;

	font_family family;
	font_style style;

	int32 count = manager->CountFontFiles();
	for (int32 i = 0; i < count; i++) {
		if (!manager->GetFontAt(i, family, style))
			break;

		BMessage* message = new BMessage(MSG_FONT_SELECTED);
		message->AddString("font family", family);
		message->AddString("font style", style);

		FontMenuItem* item = new FontMenuItem(style, family, style, message);
		item->SetTarget(target);

		bool markStyle = false;
		if (!fontMenu
			|| (fontMenu->Name()
				&& strcmp(fontMenu->Name(), family) != 0)) {
			// create new entry
			fontMenu = new BMenu(family);
			fontMenu->AddItem(item);
			menu->AddItem(fontMenu);
			// mark the menu if necessary
			if (markedFamily && strcmp(markedFamily, family) == 0) {
				if (BMenuItem* superItem = fontMenu->Superitem())
					superItem->SetMarked(true);
				markStyle = true;
			}
		} else {
			// reuse old entry
			fontMenu->AddItem(item);
		}
		// mark the item if necessary
		if (markStyle && markedStyle && strcmp(markedStyle, style) == 0)
			item->SetMarked(true);
	}

	manager->Unlock();
}


