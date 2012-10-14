/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.4
// Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software 
// is granted provided this copyright notice appears in all copies. 
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//			mcseemagg@yahoo.com
//			http://www.antigrain.com
//----------------------------------------------------------------------------


#include "FontCacheEntry.h"

#include <string.h>
#include <agg_array.h>

#include <Autolock.h>

#include "FontManager.h"

BLocker
FontCacheEntry::sUsageUpdateLock("FontCacheEntry usage lock");


class FontCacheEntry::GlyphCachePool {
	// This class needs to be defined before any inline functions, as otherwise
	// gcc2 will barf in debug mode.
	struct GlyphHashTableDefinition {
		typedef uint32		KeyType;
		typedef	GlyphCache	ValueType;

		size_t HashKey(uint32 key) const
		{
			return key;
		}

		size_t Hash(GlyphCache* value) const
		{
			return value->glyph_index;
		}

		bool Compare(uint32 key, GlyphCache* value) const
		{
			return value->glyph_index == key;
		}

		GlyphCache*& GetLink(GlyphCache* value) const
		{
			return value->hash_link;
		}
	};
public:
	enum block_size_e { block_size = 16384-16 };

	GlyphCachePool()
	{
	}

	~GlyphCachePool()
	{
		GlyphCache* glyph = fGlyphTable.Clear(true);
		while (glyph != NULL) {
			GlyphCache* next = glyph->hash_link;
			delete glyph;
			glyph = next;
		}
	}

	status_t Init()
	{
		return fGlyphTable.Init();
	}

	const GlyphCache* FindGlyph(uint32 glyphCode) const
	{
		return fGlyphTable.Lookup(glyphIndex);
	}

	GlyphCache* CacheGlyph(uint32 glyphIndex,
		uint32 dataSize, glyph_data_type dataType, const agg::rect_i& bounds,
		float advanceX, float advanceY, float insetLeft, float insetRight)
	{
		GlyphCache* glyph = fGlyphTable.Lookup(glyphIndex);
		if (glyph != NULL)
			return NULL;

		glyph = new(std::nothrow) GlyphCache(glyphIndex, dataSize, dataType,
			bounds, advanceX, advanceY, insetLeft, insetRight);
		if (glyph == NULL || glyph->data == NULL) {
			delete glyph;
			return NULL;
		}

		// TODO: The HashTable grows without bounds. We should cleanup
		// older entries from time to time.

		fGlyphTable.Insert(glyph);

		return glyph;
	}

private:
	typedef BOpenHashTable<GlyphHashTableDefinition> GlyphTable;

	GlyphTable	fGlyphTable;
};

// #pragma mark -

// constructor
FontCacheEntry::FontCacheEntry()
	: fGlyphCache(new(std::nothrow) GlyphCachePool())
	, fEngine()
	, fLastUsedTime(LONGLONG_MIN)
	, fUseCounter(0)
{
}

// destructor
FontCacheEntry::~FontCacheEntry()
{
//printf("~FontCacheEntry()\n");
	delete fGlyphCache;
}

// Init
bool
FontCacheEntry::Init(const Font& font, bool forceOutline)
{
	// load the font file in the font engine
	font_family family;
	font_style style;
	font.GetFamilyAndStyle(&family, &style);

//printf("FontCacheEntry::Init(%s/%s, outline: %d)\n",
//	family, style, forceOutline);

	BAutolock _(FontManager::Default());

	const char* fontFilePath
		= FontManager::Default()->FontFileFor(family, style);

	if (fontFilePath == NULL)
		return false;

	glyph_rendering renderingType = glyph_ren_native_gray8;
	if (forceOutline || font.Rotation() != 0.0 || font.Shear() != 90.0)
		renderingType = glyph_ren_outline;

	if (!fEngine.Init(fontFilePath, 0, font.Size(), FT_ENCODING_NONE,
			renderingType, font.Hinting())) {
		printf("FontCacheEntry::Init() - some error loading font "
			"file %s/%s\n", family, style);
		return false;
	}
	if (fGlyphCache->Init() != B_OK) {
		fprintf(stderr, "FontCacheEntry::Init() - failed to allocate "
			"GlyphCache table for font file %s\n", fontFilePath);
		return false;
	}

	return true;
}

// HasGlyphs
bool
FontCacheEntry::HasGlyphs(uint32* glyphCodes, size_t count) const
{
	for (size_t i = 0; i < count; i++) {
		if (!fGlyphCache->FindGlyph(glyphCodes[i]))
			return false;
	}
	return true;
}

inline bool
render_as_space(uint32 glyphCode)
{
	// whitespace: render as space
	// as per Unicode PropList.txt: White_Space
	return (glyphCode >= 0x0009 && glyphCode <= 0x000d)
			// control characters
		|| (glyphCode == 0x0085)
			// another control
		|| (glyphCode == 0x00a0)
			// no-break space
		|| (glyphCode == 0x1680)
			// ogham space mark
		|| (glyphCode == 0x180e)
			// mongolian vowel separator
		|| (glyphCode >= 0x2000 && glyphCode <= 0x200a)
			// en quand, hair space
		|| (glyphCode >= 0x2028 && glyphCode <= 0x2029)
			// line and paragraph separators
		|| (glyphCode == 0x202f)
			// narrow no-break space
		|| (glyphCode == 0x205f)
			// medium math space
		|| (glyphCode == 0x3000)
			// ideographic space
		;
}

inline bool
render_as_zero_width(uint32 glyphCode)
{
	// ignorable chars: render as invisible
	// as per Unicode DerivedCoreProperties.txt: Default_Ignorable_Code_Point
	return (glyphCode == 0x00ad)
			// soft hyphen
		|| (glyphCode == 0x034f)
			// combining grapheme joiner
		|| (glyphCode >= 0x115f && glyphCode <= 0x1160)
			// hangul fillers
		|| (glyphCode >= 0x17b4 && glyphCode <= 0x17b5)
			// ignorable khmer vowels
		|| (glyphCode >= 0x180b && glyphCode <= 0x180d)
			// variation selectors
		|| (glyphCode >= 0x200b && glyphCode <= 0x200f)
			// zero width space, cursive joiners, ltr marks
		|| (glyphCode >= 0x202a && glyphCode <= 0x202e)
			// left to right embed, override
		|| (glyphCode >= 0x2060 && glyphCode <= 0x206f)
			// word joiner, invisible math operators, reserved
		|| (glyphCode == 0x3164)
			// hangul filler
		|| (glyphCode >= 0xfe00 && glyphCode <= 0xfe0f)
			// variation selectors
		|| (glyphCode == 0xfeff)
			// zero width no-break space
		|| (glyphCode == 0xffa0)
			// halfwidth hangul filler
		|| (glyphCode >= 0xfff0 && glyphCode <= 0xfff8)
			// reserved
		|| (glyphCode >= 0x1d173 && glyphCode <= 0x1d17a)
			// musical symbols
		|| (glyphCode >= 0xe0000 && glyphCode <= 0xe01ef)
			// variation selectors, tag space, reserved
		;
}

// Glyph
const GlyphCache*
FontCacheEntry::Glyph(uint32 glyphCode)
{
	// TODO: See Haiku version. It appears to have split Glyph()
	// into CachedGlyph() and CreateGlyph().
	const GlyphCache* glyph = fGlyphCache->FindGlyph(glyphCode);
	if (glyph != NULL)
		return glyph;

	if (fEngine.PrepareGlyph(glyphCode)) {
		glyph = fGlyphCache->CacheGlyph(glyphCode,
			fEngine.GlyphIndex(), fEngine.DataSize(),
			fEngine.DataType(), fEngine.Bounds(),
			fEngine.AdvanceX(), fEngine.AdvanceY());

		fEngine.WriteGlyphTo(glyph->data);

		return glyph;
	}

	return NULL;
}

// InitAdaptors
void
FontCacheEntry::InitAdaptors(const GlyphCache* glyph,
	double x, double y, GlyphMonoAdapter& monoAdapter,
	GlyphGray8Adapter& gray8Adapter, GlyphPathAdapter& pathAdapter,
	double scale)
{
	if (!glyph)
		return;

	switch(glyph->data_type) {
		case glyph_data_mono:
			monoAdapter.init(glyph->data, glyph->data_size, x, y);
			break;

		case glyph_data_gray8:
			gray8Adapter.init(glyph->data, glyph->data_size, x, y);
			break;

		case glyph_data_outline:
			pathAdapter.init(glyph->data, glyph->data_size, x, y, scale);
			break;

		default:
			break;
	}
}

// GetKerning
bool
FontCacheEntry::GetKerning(uint16 glyphCode1, uint16 glyphCode2,
	double* x, double* y)
{
	return fEngine.GetKerning(glyphCode1, glyphCode2, x, y);
}

// GenerateSignature
/*static*/ void
FontCacheEntry::GenerateSignature(char* signature, const Font& font,
	bool forceOutline)
{
	// TODO: read more of these from the font
	FT_Encoding charMap = FT_ENCODING_NONE;

	font_family family;
	font_style style;
	font.GetFamilyAndStyle(&family, &style);

	int faceIndex = 0;

	glyph_rendering renderingType = glyph_ren_native_gray8;
	if (font.Rotation() != 0.0 || font.Shear() != 90.0)
		renderingType = glyph_ren_outline;

	sprintf(signature, "%s%s_%u_%d_%d_%.3f_%d_%d", family, style, charMap,
		faceIndex, int(renderingType), font.Size(), font.Hinting(),
		forceOutline);
}

// UpdateUsage
void
FontCacheEntry::UpdateUsage()
{
	// this is a static lock to prevent usage of too many semaphores,
	// but on the other hand, it is not so nice to be using a lock
	// here at all
	// the hope is that the time is so short to hold this lock, that
	// there is not much contention
	BAutolock _(sUsageUpdateLock);

	fLastUsedTime = system_time();
	fUseCounter++;
}

