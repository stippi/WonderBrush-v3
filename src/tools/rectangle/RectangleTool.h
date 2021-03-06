/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef RECTANGLE_TOOL_H
#define RECTANGLE_TOOL_H

#include "Tool.h"

class RectangleTool : public Tool {
public:
								RectangleTool();
	virtual						~RectangleTool();

	// save state
	virtual	status_t			SaveSettings(BMessage* message);
	virtual	status_t			LoadSettings(BMessage* message);

	virtual	const char*			ShortHelpMessage();

			enum {
				CORNER_RADIUS = 0,
			};

	// apply or cancel the changes of more complex editing
	virtual	status_t			Confirm();
	virtual	status_t			Cancel();

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

#endif	// RECTANGLE_TOOL_H
