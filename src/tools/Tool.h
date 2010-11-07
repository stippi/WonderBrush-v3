/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#ifndef TOOL_H
#define TOOL_H

#include <Handler.h>

class BMessage;
class Document;
class IconButton;
class Selection;
class StateView;
class ToolConfigView;
class ViewState;

// NOTE: A Tool object is added to the MainWindow,
// which switches between tools. Each tool is also
// added to the BWindows handlers, so that BMessages
// can be sent to it.

class Tool : public BHandler {
public:
								Tool(const char* name = NULL);
	virtual						~Tool();

	// save state
	virtual	status_t			SaveSettings(BMessage* message);
	virtual	status_t			LoadSettings(BMessage* message);

	// GUI
			ViewState*			ToolViewState(StateView* view,
									Document* document, Selection* selection);
	inline	ViewState*			ToolViewState() const
									{ return fViewState; }
			ToolConfigView*		ConfigView();
			IconButton*			Icon();

	virtual	const char*			ShortHelpMessage();

	// apply or cancel the changes of
	// more complex editing
	virtual	status_t			Confirm();
	virtual	status_t			Cancel();

	virtual	void				SetOption(uint32 option, bool value);
	virtual	void				SetOption(uint32 option, float value);
	virtual	void				SetOption(uint32 option, int32 value);
	// TODO: More overloaded versions.

protected:
	virtual	ViewState*			MakeViewState(StateView* view,
									Document* document,
									Selection* selection) = 0;
	virtual	ToolConfigView*		MakeConfigView() = 0;
	virtual	IconButton*			MakeIcon() = 0;

			ViewState*			fViewState;
			ToolConfigView*		fConfigView;
			IconButton*			fIcon;
};

#endif	// TOOL_H
