#ifndef TEXT_TOOL_CONFIG_VIEW_H
#define TEXT_TOOL_CONFIG_VIEW_H


#include "ToolConfigView.h"


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
			void				_SetValue(BTextControl* control,
									float value) const;
			float				_Value(BTextControl* control) const;

			void				_PopulateFontMenu(BMenu* menu,
									BHandler* target,
									const char* markedFamily,
									const char* markedStyle);

			double				_FromLinearSize(double value) const;
			double				_ToLinearSize(double value) const;

private:
			Ui::TextToolConfigView*	fUi;
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
