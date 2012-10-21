/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef FONT_CACHE_H
#define FONT_CACHE_H

#include "RWLocker.h"
#include "TextRenderer.h"

class FontCache : public RWLocker {
public:
	FontCache(int dpiX, int dpiY);
	virtual ~FontCache();

	static FontCache* getInstance();

	inline FontEngine& getFontEngine()
	{
		return fFontEngine;
	}

	inline FontManager& getFontManager()
	{
		return fFontManager;
	}

private:
	static FontCache	sDefaultInstance;

	FontEngine			fFontEngine;
	FontManager			fFontManager;
};

#endif // FONT_CACHE_H
