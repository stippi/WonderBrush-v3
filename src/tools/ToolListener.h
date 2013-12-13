/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef TOOL_LISTENER_H
#define TOOL_LISTENER_H

#include <SupportDefs.h>

class ToolListener {
public:
								ToolListener();
	virtual						~ToolListener();

	virtual	void				ConfirmableEditStarted() = 0;
	virtual	void				ConfirmableEditFinished() = 0;
};

#endif	// TOOL_LISTENER_H
