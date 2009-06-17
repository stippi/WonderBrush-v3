/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */

#include "ToolConfigView.h"

#include <String.h>

#include "Tool.h"

// constructor
ToolConfigView::ToolConfigView(::Tool* tool)
	: BView(BRect(0, 0, 40, 20), NULL, B_FOLLOW_ALL, 0),
	  fTool(tool)
{
	BString name(tool->Name());
	name << " config view";
	SetName(name.String());
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
