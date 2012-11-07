/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>
 * Copyright 2012, Ingo Weinhold <ingo_weinhold@gmx.de>
 * All rights reserved.
 */


#include "TextToolConfigView.h"
#include "ui_TextToolConfigView.h"

#include <Font.h>

#include <QApplication>

#include "FontRegistry.h"
#include "TextTool.h"
#include "TextToolState.h"


enum {
	MSG_FONT_SELECTED		= 'fnsl'
};


TextToolConfigView::TextToolConfigView(::Tool* tool)
	:
	ToolConfigView(tool),
	fUi(new Ui::TextToolConfigView),
	fText(),
	fAnchor(0),
	fCaret(0),
	fNotificationsEnabled(true)
{
	fUi->setupUi(this);

	connect(fUi->sizeSlider, SIGNAL(valueChanged(int)),
		SLOT(_SizeSliderChanged()));
	connect(fUi->sizeEdit, SIGNAL(valueChanged(double)),
		SLOT(_SizeEditChanged()));
	connect(fUi->textEdit, SIGNAL(textChanged()),
		SLOT(_TextChanged()));
	connect(fUi->textEdit, SIGNAL(selectionChanged()),
		SLOT(_TextSelectionChanged()));
	connect(fUi->subpixelsCheckBox, SIGNAL(stateChanged(int)),
		SLOT(_SubpixelsChanged()));
}


TextToolConfigView::~TextToolConfigView()
{
	delete fUi;
}


void
TextToolConfigView::AttachedToWindow()
{
	FontRegistry* fontRegistry = FontRegistry::Default();
	if (fontRegistry->LockWithTimeout(3000) == B_OK) {
		_PopulateFontPopup(NULL, NULL);
		fontRegistry->Unlock();
	}
	fontRegistry->StartWatchingAll(this);

	fUi->fontPopup->SetTarget(this);
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
			_PopulateFontPopup(NULL, NULL);
			break;

		case MSG_FONT_SELECTED:
		{
			const char* family;
			if (message->FindString("font family", &family) != B_OK)
				break;

			const char* style;
			if (message->FindString("font style", &style) != B_OK)
				break;

			((TextTool*)fTool)->SetFont(family, style);
			break;
		}

		case MSG_LAYOUT_CHANGED:
		{
			fNotificationsEnabled = false;

			float size;
			if (message->FindFloat("size", &size) == B_OK) {
				fUi->sizeSlider->setValue((int)_ToLinearSize(size));
				_SetValue(fUi->sizeEdit, size);
			}

			const char* text;
			if (message->FindString("text", &text) == B_OK) {
				// save the selection
				int anchor;
				int caret;
				_GetSelection(anchor, caret);

				// set text
				fText = QString::fromUtf8(text);
				fUi->textEdit->setText(fText);

				// reset the selection
				_SetSelection(anchor, caret);
			}

			const char* family;
			const char* style;
			if (message->FindString("family", &family) == B_OK
				&& message->FindString("style", &style) == B_OK) {
				fUi->fontPopup->SetFamilyAndStyle(family, style);
			}

			fNotificationsEnabled = true;

			break;
		}

		case MSG_SET_SELECTION:
		{
			int32 anchor;
			int32 caret;
			if (message->FindInt32("anchor", &anchor) == B_OK
				&& message->FindInt32("caret", &caret) == B_OK) {
				fNotificationsEnabled = false;
				_SetSelection(anchor, caret);
				fNotificationsEnabled = true;
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
	setEnabled(enable);
}


// #pragma mark - private


void
TextToolConfigView::_SetValue(QDoubleSpinBox* control, float value) const
{
#if 0
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
#endif
	control->setValue(value);
	// TODO: Keep selection!

}


void
TextToolConfigView::_PopulateFontPopup(const char* markedFamily,
	const char* markedStyle)
{
	FontRegistry* manager = FontRegistry::Default();

	if (!manager->Lock())
		return;

	fUi->fontPopup->MakeEmpty();

	int32 count = manager->CountFontFiles();
	for (int32 i = 0; i < count; i++) {
		font_family family;
		font_style style;
		if (!manager->GetFontAt(i, family, style))
			break;

		BMessage* message = new BMessage(MSG_FONT_SELECTED);
		message->AddString("font family", family);
		message->AddString("font style", style);

		fUi->fontPopup->AddFont(family, style, message);
	}

	if (markedFamily != NULL && markedStyle != NULL) {
		fUi->fontPopup->SetFamilyAndStyle(markedFamily, markedStyle);
	} else {
		QFont font = QApplication::font();
		fUi->fontPopup->SetFamilyAndStyle(font.family().toUtf8().data(),
			font.styleName().toUtf8().data());
	}

	manager->Unlock();
}


void
TextToolConfigView::_GetSelection(int& anchor, int& caret)
{
	QTextCursor cursor(fUi->textEdit->textCursor());
	anchor = cursor.anchor();
	caret = cursor.position();
}


void
TextToolConfigView::_SetSelection(int anchor, int caret)
{
	int documentLength = fText.length();
	fAnchor = std::min(anchor, documentLength);
	fCaret = std::min(caret, documentLength);

	QTextCursor cursor(fUi->textEdit->document());
	cursor.setPosition(fAnchor);
	if (fCaret != fAnchor)
		cursor.setPosition(fCaret, QTextCursor::KeepAnchor);
	fUi->textEdit->setTextCursor(cursor);
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


void
TextToolConfigView::_SizeSliderChanged()
{
	if (fNotificationsEnabled) {
		float value = (float)_FromLinearSize(fUi->sizeSlider->value());

		fNotificationsEnabled = false;
		fUi->sizeEdit->setValue(value);
		fNotificationsEnabled = true;

		fTool->SetOption(TextTool::SIZE, value);
	}
}


void
TextToolConfigView::_SizeEditChanged()
{
	if (fNotificationsEnabled) {
		float value = (float)fUi->sizeEdit->value();

		fNotificationsEnabled = false;
		fUi->sizeSlider->setValue((int)_ToLinearSize(value));
		fNotificationsEnabled = true;

		fTool->SetOption(TextTool::SIZE, value);
	}
}


void
TextToolConfigView::_TextChanged()
{
	int selectionStart = std::min(fAnchor, fCaret);
	int selectionEnd = std::max(fAnchor, fCaret);
	_GetSelection(fAnchor, fCaret);

	QString newText = fUi->textEdit->document()->toPlainText();
	if (newText == fText)
		return;

	if (!fNotificationsEnabled) {
		fText = newText;
		return;
	}

	// Determine the text range that has been replace. We just figure that from
	// the text selection.
	int replaceStart = selectionStart;
	int replaceEnd = selectionEnd;
	if (selectionStart == selectionEnd) {
		// The selection was empty. We assume that something has been inserted
		// or an adjacent substring has been removed. If something has been
		// removed, we determine the common ranges before and after the old and
		// the new selection and assume the part in between has been removed.
		if (newText.length() < fText.length()) {
			replaceStart = std::min(replaceStart, std::min(fAnchor, fCaret));
			replaceEnd = std::max(replaceEnd,
				fText.length() - newText.length() + std::max(fAnchor, fCaret));
		}
	}

	// Check whether the replacement range actually work, i.e. whether head and
	// tail of old and new text actually match.
	QString oldHead = fText.left(replaceStart);
	QString oldTail = fText.right(fText.length() - replaceEnd);
	if (oldHead.length() + oldTail.length() > newText.length()
		|| !newText.startsWith(oldHead) || !newText.endsWith(oldTail)) {
		// Mismatch. Just replace the whole text.
		oldHead.clear();
		oldTail.clear();
		replaceStart = 0;
		replaceEnd = fText.length();
	}

	// Assign the text and determine the inserted text.
	fText = newText;
	QString insertedText = newText.mid(replaceStart,
		newText.length() - replaceStart - oldTail.length());

	// Remove or replace the text range.
	int expectedAnchor;
	int expectedCaret;
	if (insertedText.isEmpty()) {
		// Remove.
		((TextTool*)fTool)->Remove(replaceStart,
			replaceEnd - replaceStart);
		expectedAnchor = expectedCaret = replaceStart;
	} else {
		// Replace. Since the tool doesn't have a replace method, we need to
		// make sure the selection covers exactly what we need to replace.
		if (replaceStart != selectionStart || replaceEnd != selectionEnd) {
			dynamic_cast<TextToolState*>(fTool->ToolViewState())
				->SelectionChanged(replaceStart, replaceEnd);
		}
		((TextTool*)fTool)->Insert(replaceStart,
			insertedText.toUtf8().data());
		expectedAnchor = replaceStart;
		expectedCaret = replaceStart + insertedText.length();
	}

	// Make sure the tool has the same selection as we do.
	if (fAnchor != expectedAnchor || fCaret != expectedCaret) {
		dynamic_cast<TextToolState*>(fTool->ToolViewState())
			->SelectionChanged(fAnchor, fCaret);
	}
}


void
TextToolConfigView::_TextSelectionChanged()
{
	// Get the text. If it has changed, we let _TextChanged() do the work.
	// Typically the selectionChanged() signal is sent before the textChanged()
	// signal.
	QString newText = fUi->textEdit->document()->toPlainText();
	if (newText != fText) {
		_TextChanged();
		return;
	}

	// get the new selection
	int anchor;
	int caret;
	_GetSelection(anchor, caret);
	if (anchor == fAnchor && caret == fCaret)
		return;

	fAnchor = anchor;
	fCaret = caret;

	// notify the tool, unless disabled
	if (fNotificationsEnabled) {
		dynamic_cast<TextToolState*>(fTool->ToolViewState())
			->SelectionChanged(fAnchor, fCaret);
	}
}


void
TextToolConfigView::_SubpixelsChanged()
{
	if (fNotificationsEnabled) {
		fTool->SetOption(TextTool::SUBPIXELS,
			fUi->subpixelsCheckBox->isChecked());
	}
}
