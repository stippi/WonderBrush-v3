/*
 * Copyright 2009-2010, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef BRUSH_TOOL_H
#define BRUSH_TOOL_H

#include "Tool.h"

class BrushTool : public Tool {
public:
								BrushTool();
	virtual						~BrushTool();

	// save state
	virtual	status_t			SaveSettings(BMessage* message);
	virtual	status_t			LoadSettings(BMessage* message);

	virtual	const char*			ShortHelpMessage();

protected:
	virtual	ViewState*			MakeViewState(StateView* view,
									Document* document, Selection* selection);
	virtual	ToolConfigView*		MakeConfigView();
	virtual	IconButton*			MakeIcon();
};

#endif	// BRUSH_TOOL_H
