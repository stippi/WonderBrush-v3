/*
 * Copyright 20012, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef TEXT_TOOL_H
#define TEXT_TOOL_H

#include "Text.h"
#include "Tool.h"

class TextTool : public Tool {
public:
								TextTool();
	virtual						~TextTool();

	// save state
	virtual	status_t			SaveSettings(BMessage* message);
	virtual	status_t			LoadSettings(BMessage* message);

	virtual	const char*			ShortHelpMessage();

			enum {
				SIZE = 0,
				SUBPIXELS,
			};

	virtual	void				SetOption(uint32 option, bool value);
	virtual	void				SetOption(uint32 option, float value);
	virtual	void				SetOption(uint32 option, int32 value);

private:
	virtual	ViewState*			MakeViewState(StateView* view,
									Document* document, Selection* selection);
	virtual	ToolConfigView*		MakeConfigView();
	virtual	IconButton*			MakeIcon();

private:
};

#endif	// TEXT_TOOL_H
