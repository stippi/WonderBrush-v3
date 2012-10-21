/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#include "FontCache.h"


FontCache
FontCache::sDefaultInstance(72, 72);


FontCache::FontCache(int dpiX, int dpiY)
	:
	RWLocker(),
	fFontEngine(),
	fFontManager(fFontEngine, 128)
{
	fFontEngine.flip_y(true);
	fFontEngine.resolution(dpiX, dpiY);
}


FontCache::~FontCache()
{
}


FontCache*
FontCache::getInstance()
{
	return &sDefaultInstance;
}
