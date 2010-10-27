/*
 * Copyright 2006-2010, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "ToolConfigView.h"

#include <String.h>

#include "Tool.h"

// constructor
ToolConfigView::ToolConfigView(::Tool* tool)
	: BView(NULL, B_WILL_DRAW),
	  fTool(tool)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	BString name(tool->Name());
	name << " config view";
	SetName(name.String());

	SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));
}

// destructor
ToolConfigView::~ToolConfigView()
{
}

// #pragma mark -

// UpdateStrings
void
ToolConfigView::UpdateStrings()
{
}

// SetActive
void
ToolConfigView::SetActive(bool active)
{
}

// SetEnabled
void
ToolConfigView::SetEnabled(bool enable)
{
}
