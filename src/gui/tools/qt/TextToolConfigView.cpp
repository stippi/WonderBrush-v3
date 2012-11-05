#include "TextToolConfigView.h"
#include "ui_TextToolConfigView.h"


TextToolConfigView::TextToolConfigView(::Tool* tool)
	:
	ToolConfigView(tool),
	fUi(new Ui::TextToolConfigView)
{
	fUi->setupUi(this);
}


TextToolConfigView::~TextToolConfigView()
{
	delete fUi;
}


void
TextToolConfigView::AttachedToWindow()
{
	fSizeSlider->SetTarget(this);
	fSizeTextControl->SetTarget(this);
	fSubpixels->SetTarget(this);
	fTextView->SetTarget(this);

	FontRegistry* fontRegistry = FontRegistry::Default();
	if (fontRegistry->LockWithTimeout(3000) == B_OK) {
		_PopulateFontMenu(fFontPopup->Menu(), this, NULL, NULL);
		fontRegistry->Unlock();
	}
	fontRegistry->StartWatchingAll(this);
}


void
TextToolConfigView::DetachedFromWindow()
{
	FontRegistry* fontRegistry = FontRegistry::Default();
	fontRegistry->StopWatchingAll(this);
}


void
TextToolConfigView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_OBSERVER_NOTICE_CHANGE:
			_PopulateFontMenu(fFontPopup->Menu(), this, NULL, NULL);
			break;

		case MSG_FONT_SELECTED:
		{
			fFontPopup->RefreshItemLabel();

			const char* family;
			if (message->FindString("font family", &family) != B_OK)
				break;

			const char* style;
			if (message->FindString("font style", &style) != B_OK)
				break;

			((TextTool*)fTool)->SetFont(family, style);
			break;
		}

		case MSG_SIZE_SLIDER:
			fTool->SetOption(TextTool::SIZE,
				(float)_FromLinearSize(fSizeSlider->Value()));
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
				fSizeSlider->SetValue(_ToLinearSize(size));
				_SetValue(fSizeTextControl, size);
			}

			const char* text;
			if (message->FindString("text", &text) == B_OK) {
				fTextView->SetNotificationsEnabled(false);
				fTextView->SetText(text);
				fTextView->SetNotificationsEnabled(true);
			}

			const char* family;
			const char* style;
			if (message->FindString("family", &family) == B_OK
				&& message->FindString("style", &style) == B_OK) {
				fFontPopup->SetFamilyAndStyle(family, style);
			}

			break;
		}

		default:
			ToolConfigView::MessageReceived(message);
			break;
	}
}


// #pragma mark -


void
TextToolConfigView::UpdateStrings()
{
}


void
TextToolConfigView::SetActive(bool active)
{
}


void
TextToolConfigView::SetEnabled(bool enable)
{
	fSizeSlider->SetEnabled(enable);
	fSizeTextControl->SetEnabled(enable);
}


// #pragma mark - private


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


float
TextToolConfigView::_Value(BTextControl* control) const
{
	return atof(control->Text());
}


void
TextToolConfigView::_PopulateFontMenu(BMenu* menu, BHandler* target,
	const char* markedFamily, const char* markedStyle)
{
	if (menu == NULL)
		return;

	font_family defaultFamily;
	font_style defaultStyle;
	if (markedFamily == NULL || markedStyle == NULL) {
		be_plain_font->GetFamilyAndStyle(&defaultFamily, &defaultStyle);
		markedFamily = defaultFamily;
		markedStyle = defaultStyle;
	}

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
		if (fontMenu == NULL
			|| (fontMenu->Name()
				&& strcmp(fontMenu->Name(), family) != 0)) {
			// create new entry
			fontMenu = new BMenu(family);
			fontMenu->AddItem(item);
			menu->AddItem(fontMenu);
			// mark the menu if necessary
			if (markedFamily != NULL && strcmp(markedFamily, family) == 0) {
				if (BMenuItem* superItem = fontMenu->Superitem())
					superItem->SetMarked(true);
				markStyle = true;
			}
		} else {
			// reuse old entry
			fontMenu->AddItem(item);
		}
		// mark the item if necessary
		if (markStyle
			&& markedStyle != NULL && strcmp(markedStyle, style) == 0) {
			item->SetMarked(true);
		}
	}

	fFontPopup->SetFamilyAndStyle(markedFamily, markedStyle);

	manager->Unlock();
}


double
TextToolConfigView::_FromLinearSize(double value) const
{
	return 1.0 + 1023.0 * pow((value - 1) / 1023.0, 2.0);
}


double
TextToolConfigView::_ToLinearSize(double value) const
{
	return 1.0 + 1023.0 * sqrt((value - 1) / 1023.0);
}
