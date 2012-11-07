/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>
 * Copyright 2012, Ingo Weinhold <ingo_weinhold@gmx.de>
 * All rights reserved.
 */
#ifndef TEXT_TOOL_CONFIG_VIEW_H
#define TEXT_TOOL_CONFIG_VIEW_H


#include "ToolConfigView.h"


class QDoubleSpinBox;

class FontPopup;


namespace Ui {
	class TextToolConfigView;
}


class TextToolConfigView : public ToolConfigView {
	Q_OBJECT
	
public:
								TextToolConfigView(::Tool* tool);
	virtual						~TextToolConfigView();

	// ToolConfigView interface
	virtual	void				UpdateStrings();
	virtual	void				SetActive(bool active);
	virtual	void				SetEnabled(bool enable);

	// BView interface
	virtual	void				AttachedToWindow();
	virtual	void				DetachedFromWindow();
	virtual	void				MessageReceived(BMessage* message);

private:
			void				_SetValue(QDoubleSpinBox* control,
									float value) const;

			void				_PopulateFontPopup(const char* markedFamily,
									const char* markedStyle);

			void				_GetSelection(int& anchor, int& caret);
			void				_SetSelection(int anchor, int caret);

			double				_FromLinearSize(double value) const;
			double				_ToLinearSize(double value) const;

private slots:
			void				_SizeSliderChanged();
			void				_SizeEditChanged();
			void				_TextChanged();
			void				_TextSelectionChanged();
			void				_SubpixelsChanged();

private:
			Ui::TextToolConfigView*	fUi;

			QString				fText;
			int					fAnchor;
			int					fCaret;
			bool				fNotificationsEnabled;
};



//class TextToolConfigView : public ToolConfigView {
//public:

//private:
//			FontPopup*			fFontPopup;

//			BStringView*		fSizeLabel;
//			BSlider*			fSizeSlider;
//			BTextControl*		fSizeTextControl;

//			BCheckBox*			fSubpixels;

//			NotifyingTextView*	fTextView;
//};


#endif // TEXT_TOOL_CONFIG_VIEW_H
