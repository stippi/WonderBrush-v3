/*
 * Copyright 2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#include "FontCache.h"


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
	static FontCache cache(72, 72);
	return &cache;
}


void
FontCache::setFontFolder(const char* path)
{
	fFontFolder = path;
}
	

const BString&
FontCache::getFontFolder() const
{
	return fFontFolder;
}


BString
FontCache::resolveFont(const char* fontFilePath) const
{
	BString resolvedFontPath(fontFilePath);
	if (resolvedFontPath.Length() == 0)
		return resolvedFontPath;

	if (resolvedFontPath.String()[0] != '/' && fFontFolder.Length() > 0) {
		resolvedFontPath = getFontFolder();
		resolvedFontPath << "/" << fontFilePath;
	}
	
	return resolvedFontPath;
}
