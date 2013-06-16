/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef RECTANGLE_TOOL_CONFIG_VIEW_H
#define RECTANGLE_TOOL_CONFIG_VIEW_H

#include "ToolConfigView.h"

class BSlider;
class BTextControl;

class RectangleToolConfigView : public ToolConfigView {
public:
								RectangleToolConfigView(::Tool* tool);
	virtual						~RectangleToolConfigView();

	// ToolConfigView interface
	virtual	void				UpdateStrings();
	virtual	void				SetActive(bool active);
	virtual	void				SetEnabled(bool enable);

	// BView interface
	virtual	void				AttachedToWindow();
	virtual	void				DetachedFromWindow();
	virtual	void				MessageReceived(BMessage* message);

private:
			double				_FromLinearSize(double value) const;
			double				_ToLinearSize(double value) const;

private:
			BSlider*			fRoundCornerRadiusSlider;
			BTextControl*		fRoundCornerRadiusTextControl;
};

#endif // RECTANGLE_TOOL_CONFIG_VIEW_H
