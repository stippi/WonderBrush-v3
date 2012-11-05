/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef TRANSFORM_TOOL_CONFIG_VIEW_H
#define TRANSFORM_TOOL_CONFIG_VIEW_H

#include "ToolConfigView.h"

class BCheckBox;
class BStringView;
class BTextControl;

class TransformToolConfigView : public ToolConfigView {
public:
								TransformToolConfigView(::Tool* tool);
	virtual						~TransformToolConfigView();

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

private:
			BStringView*		fTranslateLabel;
			BStringView*		fRotateLabel;
			BStringView*		fScaleLabel;

			BTextControl*		fTranslationX;
			BTextControl*		fTranslationY;
			BTextControl*		fRotate;
			BTextControl*		fScaleX;
			BTextControl*		fScaleY;

			BCheckBox*			fSubpixels;
};

#endif // TRANSFORM_TOOL_CONFIG_VIEW_H
