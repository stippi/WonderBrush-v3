/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef PICK_TOOL_H
#define PICK_TOOL_H

#include "Tool.h"

class PickTool : public Tool {
public:
								PickTool();
	virtual						~PickTool();

	// save state
	virtual	status_t			SaveSettings(BMessage* message);
	virtual	status_t			LoadSettings(BMessage* message);

	virtual	const char*			ShortHelpMessage();

protected:
	virtual	ViewState*			MakeViewState(StateView* view,
									Document* document);
	virtual	ToolConfigView*		MakeConfigView();
	virtual	IconButton*			MakeIcon();
};

#endif	// TOOL_H
