/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef BRUSH_TOOL_CONFIG_VIEW_H
#define BRUSH_TOOL_CONFIG_VIEW_H

#include "ToolConfigView.h"

class BCheckBox;
class DualSlider;

class BrushToolConfigView : public ToolConfigView {
public:
								BrushToolConfigView(::Tool* tool);
	virtual						~BrushToolConfigView();

	// ToolConfigView interface
	virtual	void				UpdateStrings();
	virtual	void				SetActive(bool active);
	virtual	void				SetEnabled(bool enable);

	// BView interface
	virtual	void				AttachedToWindow();
	virtual	void				MessageReceived(BMessage* message);

private:
			void				_SetMinMaxOption(const BMessage* message,
									uint32 minOption, uint32 maxOption);

private:
			DualSlider*			fOpacity;
			DualSlider*			fRadius;
			DualSlider*			fHardness;
			DualSlider*			fSpacing;

			BCheckBox*			fSubpixels;
			BCheckBox*			fSolid;
			BCheckBox*			fTilt;
};

#endif // BRUSH_TOOL_CONFIG_VIEW_H
