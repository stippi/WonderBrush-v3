/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef TRANSFORM_TOOL_H
#define TRANSFORM_TOOL_H

#include "Tool.h"

class TransformTool : public Tool {
public:
								TransformTool();
	virtual						~TransformTool();

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

#endif	// TRANSFORM_TOOL_H
