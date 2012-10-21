/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#include "TextSnapshot.h"

#include <stdio.h>

#include "support.h"

#include "Text.h"
#include "RenderEngine.h"

// constructor
TextSnapshot::TextSnapshot(const Text* text)
	: StyleableSnapshot(text)
	, fOriginal(text)
	, fTextLayout(text->getTextLayout())
{
}

// destructor
TextSnapshot::~TextSnapshot()
{
}

// #pragma mark -

// Original
const Object*
TextSnapshot::Original() const
{
	return fOriginal;
}

// Sync
bool
TextSnapshot::Sync()
{
	if (StyleableSnapshot::Sync()) {
		fTextLayout = fOriginal->getTextLayout();
		return true;
	}
	return false;
}

// Render
void
TextSnapshot::Render(RenderEngine& engine, RenderBuffer* bitmap,
	BRect area) const
{
	engine.SetStyle(fStyle);
	engine.SetTransformation(LayoutedState().Matrix);
	// TODO: ...
}


