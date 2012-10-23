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

			enum {
				TRANSLATION_X = 0,
				TRANSLATION_Y,
				ROTATION,
				SCALE_X,
				SCALE_Y,
		
				SUBPIXELS,
			};

	virtual	void				SetOption(uint32 option, bool value);
	virtual	void				SetOption(uint32 option, float value);
	virtual	void				SetOption(uint32 option, int32 value);
	virtual	void				SetOption(uint32 option, const char* value);

protected:
	virtual	ViewState*			MakeViewState(StateView* view,
									Document* document, Selection* selection);
	virtual	ToolConfigView*		MakeConfigView();
	virtual	IconButton*			MakeIcon();
};

#endif	// TRANSFORM_TOOL_H
