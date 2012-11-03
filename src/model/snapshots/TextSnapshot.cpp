/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#include "TextSnapshot.h"

#include <stdio.h>

#include "support.h"

#include "FontCache.h"
#include "RenderBuffer.h"
#include "RenderEngine.h"
#include "Text.h"
#include "TextRenderer.h"

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

	TextRenderer renderer(FontCache::getInstance());
	renderer.attachToBuffer(
		bitmap->Bits(),
		bitmap->Width(),
		bitmap->Height(),
		bitmap->BytesPerRow()
	);
	renderer.setTransformation(LayoutedState().Matrix);
	renderer.setGrayScale(true);

	if (FontCache::getInstance()->ReadLock()) {
		renderer.drawText(
			const_cast<TextLayout*>(&fTextLayout),
			0, 0, -1, -1,
			Color(255, 255, 255), Color(80, 128, 255),
			TEXT_TRANSPARENT
		);
		FontCache::getInstance()->ReadUnlock();
	}
}


