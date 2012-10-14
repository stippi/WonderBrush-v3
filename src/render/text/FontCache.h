/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef FONT_CACHE_H
#define FONT_CACHE_H

#include "Font.h"
#include "FontCacheEntry.h"
#include "HashMap.h"
#include "HashString.h"
#include "RWLocker.h"


class FontCache : public RWLocker {
 public:
								FontCache();
	virtual						~FontCache();

	// global instance
	static	FontCache*			Default();

			FontCacheEntry*		FontCacheEntryFor(const Font& font,
									bool forceOutline = true);
			void				Recycle(FontCacheEntry* entry);

 private:
			void				_ConstrainEntryCount();

	static	FontCache			sDefaultInstance;

	typedef HashMap<HashString, FontCacheEntry*> FontMap;

			FontMap				fFontCacheEntries;
};

#endif // FONT_CACHE_H
