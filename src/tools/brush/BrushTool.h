/*
 * Copyright 2009-2010, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef BRUSH_TOOL_H
#define BRUSH_TOOL_H

#include "Brush.h"
#include "Tool.h"

class BrushTool : public Tool {
public:
								BrushTool();
	virtual						~BrushTool();

	// save state
	virtual	status_t			SaveSettings(BMessage* message);
	virtual	status_t			LoadSettings(BMessage* message);

	virtual	const char*			ShortHelpMessage();

			enum {
				OPACITY_MIN = 0,
				OPACITY_MAX,
				OPACITY_CONTROLLED,

				RADIUS_MIN,
				RADIUS_MAX,
				RADIUS_CONTROLLED,

				HARDNESS_MIN,
				HARDNESS_MAX,
				HARDNESS_CONTROLLED,

				SPACING,
				SOLID,
				SUBPIXELS,
				TILT_CONTROLLED,
			};

	virtual	void				SetOption(uint32 option, bool value);
	virtual	void				SetOption(uint32 option, float value);
	virtual	void				SetOption(uint32 option, int32 value);
	virtual	void				SetOption(uint32 option, const char* value);

private:
	virtual	ViewState*			MakeViewState(StateView* view,
									Document* document, Selection* selection,
									CurrentColor* color);
	virtual	ToolConfigView*		MakeConfigView();
	virtual	IconButton*			MakeIcon();

private:
			Brush				fBrush;
};

#endif	// BRUSH_TOOL_H
