/*
 * Copyright 20012, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef PATH_TOOL_H
#define PATH_TOOL_H

#include "Tool.h"

class PathTool : public Tool {
public:
								PathTool();
	virtual						~PathTool();

	// save state
	virtual	status_t			SaveSettings(BMessage* message);
	virtual	status_t			LoadSettings(BMessage* message);

	virtual	const char*			ShortHelpMessage();

	virtual	void				SetOption(uint32 option, bool value);
	virtual	void				SetOption(uint32 option, float value);
	virtual	void				SetOption(uint32 option, int32 value);
	virtual void				SetOption(uint32 option, const char* value);

private:
	virtual	ViewState*			MakeViewState(StateView* view,
									Document* document, Selection* selection,
									CurrentColor* color);
	virtual	ToolConfigView*		MakeConfigView();
	virtual	IconButton*			MakeIcon();

private:
};

#endif	// PATH_TOOL_H
