/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef TEXT_TOOL_CONFIG_VIEW_H
#define TEXT_TOOL_CONFIG_VIEW_H

#include "ToolConfigView.h"

class BCheckBox;
class BMenu;
class BSlider;
class BStringView;
class BTextControl;
class FontPopup;
class NotifyingTextView;

class TextToolConfigView : public ToolConfigView {
public:
								TextToolConfigView(::Tool* tool);
	virtual						~TextToolConfigView();

	// ToolConfigView interface
	virtual	void				UpdateStrings();
	virtual	void				SetActive(bool active);
	virtual	void				SetEnabled(bool enable);

	// BView interface
	virtual	void				AttachedToWindow();
	virtual	void				MessageReceived(BMessage* message);

private:
			void				_SetValue(BTextControl* control,
									float value) const;
			float				_Value(BTextControl* control) const;

			void				_PolulateFontMenu(BMenu* menu,
									BHandler* target,
									const char* markedFamily,
									const char* markedStyle);

private:
			FontPopup*			fFontPopup;

			BSlider*			fSizeSlider;
			BTextControl*		fSizeTextControl;

			BCheckBox*			fSubpixels;

			NotifyingTextView*	fTextView;
};

#endif // TEXT_TOOL_CONFIG_VIEW_H
