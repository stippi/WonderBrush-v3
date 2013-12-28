/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef FONT_CACHE_H
#define FONT_CACHE_H

#include <String.h>

#include "RWLocker.h"
#include "TextRenderer.h"

class FontCache : public RWLocker {
public:
	FontCache(int dpiX, int dpiY);
	virtual ~FontCache();

	static FontCache* getInstance();

	inline TextRenderer::FontEngine& getFontEngine()
	{
		return fFontEngine;
	}

	inline TextRenderer::FontManager& getFontManager()
	{
		return fFontManager;
	}

private:
	TextRenderer::FontEngine	fFontEngine;
	TextRenderer::FontManager	fFontManager;
};

#endif // FONT_CACHE_H
