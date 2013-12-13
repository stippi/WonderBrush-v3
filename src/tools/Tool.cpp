/*
 * Copyright 2006-2013, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "Tool.h"

#include <stdio.h>

#include "ToolListener.h"
#include "ViewState.h"

// constructor
Tool::Tool(const char* name)
	:
	BHandler(name),
	fViewState(NULL),
	fConfigView(NULL),
	fIcon(NULL)
{
}

// destructor
Tool::~Tool()
{
	delete fViewState;
	// NOTE: the GUI stuff is deleted by the window
	// to which it has been attached
}

// #pragma mark -

// SaveSettings
status_t
Tool::SaveSettings(BMessage* message)
{
	if (message != NULL)
		return B_OK;
	return B_BAD_VALUE;
}

// LoadSettings
status_t
Tool::LoadSettings(BMessage* message)
{
	if (message != NULL)
		return B_OK;
	return B_BAD_VALUE;
}

// #pragma mark -

// ToolViewState
ViewState*
Tool::ToolViewState(StateView* view, Document* document, Selection* selection,
	CurrentColor* color)
{
	if (fViewState == NULL)
		fViewState = MakeViewState(view, document, selection, color);
	return fViewState;
}

// ConfigView
ToolConfigView*
Tool::ConfigView()
{
	if (fConfigView == NULL)
		fConfigView = MakeConfigView();
	return fConfigView;
}

// Icon
IconButton*
Tool::Icon()
{
	if (fIcon == NULL)
		fIcon = MakeIcon();
	return fIcon;
}

// ShortHelpMessage
const char*
Tool::ShortHelpMessage()
{
	return NULL;
}

// #pragma mark -

// Confirm
status_t
Tool::Confirm()
{
	return B_OK;
}

// Cancel
status_t
Tool::Cancel()
{
	return B_OK;
}

// AddListener
bool
Tool::AddListener(ToolListener* listener)
{
	return fToolListeners.Add(listener);
}

// RemoveListener
void
Tool::RemoveListener(ToolListener* listener)
{
	fToolListeners.Remove(listener);
}

// #pragma mark -

// SetOption
void
Tool::SetOption(uint32 option, bool value)
{
}

// SetOption
void
Tool::SetOption(uint32 option, float value)
{
}

// SetOption
void
Tool::SetOption(uint32 option, int32 value)
{
}

// SetOption
void
Tool::SetOption(uint32 option, const char* value)
{
}

// NotifyConfirmableEditStarted
void
Tool::NotifyConfirmableEditStarted()
{
	int count = fToolListeners.CountItems();
	for (int i = 0; i < count; i++)
		fToolListeners.ItemAtFast(i)->ConfirmableEditStarted();
}

// NotifyConfirmableEditFinished
void
Tool::NotifyConfirmableEditFinished()
{
	int count = fToolListeners.CountItems();
	for (int i = 0; i < count; i++)
		fToolListeners.ItemAtFast(i)->ConfirmableEditFinished();
}


