/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "TextBlockRenderer.h"

#include <math.h>
#include <malloc.h>
#include <new>
#include <stdio.h>
#include <string.h>

#include <ByteOrder.h>
#include <Entry.h>
#include <Message.h>
#include <Path.h>
#include <UTF8.h>

#include <agg_basics.h>
#include <agg_bounding_rect.h>
#include <agg_conv_segmentator.h>
#include <agg_conv_transform.h>
#include <agg_trans_affine.h>

#include "support.h"

#include "CommonPropertyIDs.h"
#include "FontManager.h"

using std::nothrow;

#define FLIP_Y true
#define DEFAULT_UNI_CODE_BUFFER_SIZE	1024

// #pragma mark -

// word
struct TextBlockRenderer::word {
	word(double x, int32 startGlyphIndex)
		: x_offset(x),
		  length(0.0),
		  offsets(NULL),
		  indices(NULL),
		  start_glyph_index(startGlyphIndex),
		  logical_count(0),
		  physical_count(0)
	{
	}
	~word()
	{
		free((void*)offsets);
		free((void*)indices);
	}
	bool add(uint16 index, double width)
	{
		if (!set_count(logical_count + 1))
			return false;
		offsets[logical_count - 1] = width;
		indices[logical_count - 1] = index;
		length += width;
		return true;
	}
	double end()
	{
		return x_offset + length;
	}
	bool set_count(int32 count)
	{
		if (count > physical_count) {
			physical_count = ((count + 15) / 16) * 16;
			double* _offsets = (double*)realloc((void*)offsets,
				physical_count * sizeof(double));
			uint16* _indices = (uint16*)realloc((void*)indices,
				physical_count * sizeof(uint16));
			if (!_offsets || !_indices) {
				free(_offsets);
				free(_indices);
				return false;
			}
			offsets = _offsets;
			indices = _indices;
		}
		logical_count = count;
		return true;
	}
	void print_to_stream() const
	{
		printf("    start glyph index: %ld\n", start_glyph_index);
		printf("    offset: %.2f\n", x_offset);
		for (int32 i = 0; i < logical_count; i++)
			printf("     width: %.2f, '%c'\n", offsets[i], (char)indices[i]);
	}

	double		x_offset;
	double		length;
	double*		offsets;
	uint16*		indices;
	int32		start_glyph_index;
	int32		logical_count;
	int32		physical_count;
};

// line
struct TextBlockRenderer::line {
	line(double y, int32 startGlyphIndex)
		: y_offset(y),
		  words(NULL),
		  start_glyph_index(startGlyphIndex),
		  logical_count(0),
		  physical_count(0),
		  terminated(false)
	{
	}
	~line()
	{
		for (int32 i = 0; i < logical_count; i++)
			delete words[i];
		free((void*)words);
	}
	bool add(word* w)
	{
		if (!set_count(logical_count + 1))
			return false;
		words[logical_count - 1] = w;
		return true;
	}
	double length()
	{
		double l = 0.0;
		if (logical_count > 0)
			l = words[logical_count - 1]->x_offset + words[logical_count - 1]->length;
		return l;
	}
	bool set_count(int32 count)
	{
		if (count > physical_count) {
			physical_count = ((count + 15) / 16) * 16;
			word** _words = (word**)realloc((void*)words,
				physical_count * sizeof(word*));
			if (!_words)
				return false;
			words = _words;
		}
		logical_count = count;
		return true;
	}
	void justify(uint32 alignment, double maxWidth, bool hinted)
	{
		if (logical_count >= 1) {
			switch (alignment) {
				case ALIGN_CENTER:
				case ALIGN_END: {
					double start = words[0]->x_offset;
					double end = maxWidth - words[logical_count - 1]->end();
					double shift = end - start;
					if (alignment == ALIGN_CENTER)
						shift /= 2.0;
					if (hinted)
						shift = floorf(shift + 0.5);
					for (int32 i = 0; i < logical_count; i++) {
						words[i]->x_offset += shift;
					}
					break;
				}
				case ALIGN_STRETCH:
					if (!terminated) {
						double room = maxWidth - words[logical_count - 1]->end();
						if (logical_count > 1) {
							// more than one word
							if (room > maxWidth * 0.125) {
								// we have so much room, that it will look better to
								// add spacing between glyphs as well, the glyph<->space
								// ration is not perfect, but pretty good for now
								double glyphSpaceRatio = 1.0 / (double)(logical_count - 1);
								double glyphRoom = (room - maxWidth * 0.125) * glyphSpaceRatio;
								double spacingRoom = room - glyphRoom;
								spacingRoom /= (double)(logical_count - 1);
								glyphRoom /= (double)(count_glyphs() - logical_count);
								double glyphSpaceAdded = 0.0;
								for (int32 i = 0; i < logical_count; i++) {
									if (i > 0) {
										// add spacing before word
										words[i]->x_offset += glyphSpaceAdded + i * spacingRoom;
										if (hinted)
											words[i]->x_offset = floorf(words[i]->x_offset + (1.0 / (logical_count - 1) * i));
									}
									// add spacing to glyphs
									if (words[i]->logical_count > 1) {
										for (int32 c = 0; c < words[i]->logical_count - 1; c++) {
											words[i]->offsets[c] += glyphRoom;
											glyphSpaceAdded += glyphRoom;
											if (hinted)
												words[i]->offsets[c] = floorf(words[i]->offsets[c] + (1.0 / (words[i]->logical_count - 1) * c));
										}
									}
								}
							} else {
								// add room between words only
								room /= (double)(logical_count - 1);
								for (int32 i = 1; i < logical_count; i++) {
									words[i]->x_offset += i * room;
									if (hinted)
										words[i]->x_offset = floorf(words[i]->x_offset + (1.0 / (logical_count - 1) * i));
								}
							}
						} else {
							// only one word, add room between glyphs
							if (room > 0.0) {
								word* w = words[0];
								if (w->logical_count > 1) {
									room /= (double)(w->logical_count - 1);
									for (int32 i = 0; i < w->logical_count - 1; i++) {
										w->offsets[i] += room;
										if (hinted)
											w->offsets[i] = floorf(w->offsets[i] + (1.0 / (w->logical_count - 1) * i));
									}
								}
							}
						}
					}
					break;
			}
		}
	}
	uint32 count_glyphs() const
	{
		uint32 count = 0;
		for (int32 i = 0; i < logical_count; i++) {
			count += words[i]->logical_count;
		}
		return count;
	}
	void print_to_stream() const
	{
		printf("start_glyph_index: %ld\n", start_glyph_index);
		for (int32 i = 0; i < logical_count; i++) {
			printf("  word %ld\n", i);
			words[i]->print_to_stream();
		}
		if (terminated)
			printf("->return\n");
	}

	double		y_offset;
	word**		words;
	int32		start_glyph_index;
	int32		logical_count;
	int32		physical_count;
	bool		terminated;
};

// #pragma mark -

// constructor
TextBlockRenderer::TextBlockRenderer()
	: fFontCache(FontCache::Default())

	, fFont()

	, fCurves(fPathAdaptor)
	, fContour(fCurves)

	, fUnicodeBuffer((char*)malloc(DEFAULT_UNI_CODE_BUFFER_SIZE))
	, fUnicodeBufferSize(fUnicodeBuffer ? DEFAULT_UNI_CODE_BUFFER_SIZE : 0)

	, fText()
	, fParagraphInset(0.0)
	, fParagraphSpacing(1.0)
	, fLineOffset(1.0)
	, fGlyphSpacing(1.0)
	, fBlockWidth(300.0)
	, fAlignment(ALIGN_BEGIN)

	, fGlyphCount(0)
	, fBounds(0.0, 0.0, -1.0, -1.0)
	, fNeedsLayout(true)
	, fLines(4)

	, fSpaceWidth(0.0)
{
	fCurves.approximation_scale(2.0);
	fContour.auto_detect_orientation(false);
}

// destructor
TextBlockRenderer::~TextBlockRenderer()
{
	free(fUnicodeBuffer);
}

// #pragma mark -

// SetText
void
TextBlockRenderer::SetText(const char* utf8String)
{
	if (fText == utf8String)
		return;

	fText = utf8String;
	fGlyphCount = 0;
		// triggers update
}

// SetFont
void
TextBlockRenderer::SetFont(const Font& font)
{
	if (fFont != font) {
		fFont = font;

		fNeedsLayout = true;
		fSpaceWidth = 0.0;
	}
}

// SetLayout
void
TextBlockRenderer::SetLayout(float paragraphInset,
							 float paragraphSpacing,
							 float lineOffset,
							 float glyphSpacing,
							 float blockWidth,
							 uint8 alignment)
{
	if (fParagraphInset == paragraphInset
		&& fParagraphSpacing == paragraphSpacing
		&& fLineOffset == lineOffset
		&& fGlyphSpacing == glyphSpacing
		&& fBlockWidth == blockWidth
		&& fAlignment == alignment)
		return;

	fParagraphInset = paragraphInset;
	fParagraphSpacing = paragraphSpacing;
	fLineOffset = lineOffset;
	fGlyphSpacing = glyphSpacing;
	fBlockWidth = blockWidth;
	fAlignment = alignment;

	_Update();
}

// #pragma mark -

// Bounds
BRect
TextBlockRenderer::Bounds(const AffineTransform& transform)
{
	if (fGlyphCount == 0 || fNeedsLayout)
		_Update();

	return transform.TransformBounds(fBounds);
}

// GetBaselinePosition
BPoint
TextBlockRenderer::GetBaselinePosition(int32 glyphIndex)
{
	BPoint position;
	int32 lineIndex;
	int32 wordIndex;
	_GetGlyphPositionInfo(glyphIndex, &position, &lineIndex, &wordIndex);
	return position;
}

// GetBaselinePositions
void
TextBlockRenderer::GetBaselinePositions(int32 startGlyphIndex,
	int32 endGlyphIndex, BPoint* _startOffset, BPoint* _endOffset)
{
	if (_startOffset)
		*_startOffset = GetBaselinePosition(startGlyphIndex);
	if (_endOffset)
		*_endOffset = GetBaselinePosition(endGlyphIndex);
}

// GlyphIndexAt
int32
TextBlockRenderer::GlyphIndexAt(BPoint baselinePosition)
{
	_LayoutIfNeeded();

	int32 lineIndex = _FindLineAtOffset(baselinePosition.y);

	line* currentLine = fLines.ItemAt(lineIndex);
	if (!currentLine) {
		if (baselinePosition.y < 0)
			return 0;
		else
			return fGlyphCount;
	}

	return _GlyphIndexAt(currentLine, baselinePosition);
}

// FirstGlyphIndexAtLine
int32
TextBlockRenderer::FirstGlyphIndexAtLine(int32 currentGlyphIndex)
{
	_LayoutIfNeeded();
	line* currentLine = fLines.ItemAt(_FindLineContaining(currentGlyphIndex));
	if (!currentLine)
		return 0;
	return currentLine->start_glyph_index;
}

// LastGlyphIndexAtLine
int32
TextBlockRenderer::LastGlyphIndexAtLine(int32 currentGlyphIndex)
{
	_LayoutIfNeeded();
	line* nextLine = fLines.ItemAt(
		_FindLineContaining(currentGlyphIndex) + 1);
	if (!nextLine)
		return fGlyphCount;
	return nextLine->start_glyph_index - 1;
}

// NextGlyphAtLineOffset
int32
TextBlockRenderer::NextGlyphAtLineOffset(int32 glyphIndex,
	int32 lineOffset)
{
	// find the line index and the position of the glyphIndex
	BPoint position;
	int32 lineIndex;
	int32 wordIndex;
	_GetGlyphPositionInfo(glyphIndex, &position, &lineIndex, &wordIndex);

	// peek into the new line
	lineIndex += lineOffset;
	line* newLine = fLines.ItemAt(lineIndex);
	if (!newLine) {
		// no change in glyph index if there is no line above/below
		// the line at which we found the glyphIndex
		return glyphIndex;
	}

	// find a glyph index with similar horizontal offset in the new line
	position.y = newLine->y_offset;
	return _GlyphIndexAt(newLine, position);
}

// GlyphWidth
float
TextBlockRenderer::GlyphWidth(uint16 glyphIndex) const
{
	if (!fUnicodeBuffer)
		return 0.0;

	FontCacheEntry* entry = fFontCache->FontCacheEntryFor(fFont);

	if (!entry || !entry->ReadLock()) {
		fFontCache->Recycle(entry);
		return 0.0;
	}

	bool needsWriteLock
		= !entry->HasGlyphs((uint16*)fUnicodeBuffer, fGlyphCount);

	if (needsWriteLock) {
		entry->ReadUnlock();
		if (!entry->WriteLock()) {
			fFontCache->Recycle(entry);
			return 0.0;
		}
	}

	float width = 0.0;
	const GlyphCache* glyph = entry->Glyph(glyphIndex);
	if (glyph) {
		// calculate the advance offset
		double advanceX = glyph->advance_x;
		double totalAdvanceX = fGlyphSpacing * advanceX;
		if (advanceX > 0.0 && fGlyphSpacing > 1.0)
			totalAdvanceX += (fGlyphSpacing - 1.0) * fFont.Size();
		width = totalAdvanceX;
	}

	if (needsWriteLock)
		entry->WriteUnlock();
	else
		entry->ReadUnlock();

	fFontCache->Recycle(entry);

	return width;
}

// #pragma mark -

class Updater {
public:
	Updater()
		: fBounds(LONG_MAX, LONG_MAX, LONG_MIN, LONG_MIN)
	{
	}

	void AddGlyph(FontCacheEntry* entry, double x, double y,
		const GlyphCache* glyph)
	{
		const agg::rect_i& r = glyph->bounds;
		BRect glyphBounds(r.x1 + x, r.y1 + y - 1, r.x2 + x + 1,
			r.y2 + y + 1);
		if (glyphBounds.IsValid())
			fBounds = fBounds | glyphBounds;
	}

	void Finish()
	{
	}

	bool NeedsOutline() const
	{
		return false;
	}

	BRect Bounds() const
	{
		return fBounds;
	}

private:
	BRect	fBounds;
};

// GlyphRenderer
typedef FontCacheEntry::GlyphPathAdapter	PathAdaptor;
typedef FontCacheEntry::GlyphGray8Adapter	Gray8Adapter;
typedef FontCacheEntry::GlyphGray8Scanline	Gray8Scanline;
typedef FontCacheEntry::GlyphMonoAdapter	MonoAdapter;
typedef FontCacheEntry::GlyphMonoScanline	MonoScanline;

template<class Rasterizer, class Scanline, class Renderer>
class GlyphRenderer {
public:
	GlyphRenderer(
		PathAdaptor& pathAdaptor, Gray8Adapter& gray8Adaptor,
		Gray8Scanline& gray8Scanline, MonoAdapter& monoAdaptor,
		MonoScanline& monoScanline,
		Rasterizer& rasterizer, Scanline& scanline, Renderer& renderer,
		const BRect& constrainRect,
		const AffineTransform& transform, float scale)
		: fPathAdaptor(pathAdaptor)
		, fGray8Adaptor(gray8Adaptor)
		, fGray8Scanline(gray8Scanline)
		, fMonoAdaptor(monoAdaptor)
		, fMonoScanline(monoScanline)
		, fRasterizer(rasterizer)
		, fScanline(scanline)
		, fRenderer(renderer)

		, fConstrainRect(constrainRect)
		, fTransform(transform)
		, fScale(scale)
		, fTransformOffset(0.0, 0.0)
	{
		// convert constrain rect from pixel index into vector format
		fConstrainRect.right++;
		fConstrainRect.bottom++;
		fRasterizer.reset();
		fRasterizer.clip_box(fConstrainRect.left, fConstrainRect.top,
							 fConstrainRect.right, fConstrainRect.bottom);

		// for when we bypass the transformation pipeline
		transform.Transform(&fTransformOffset);
	}

	void AddGlyph(FontCacheEntry* entry, double x, double y,
		const GlyphCache* glyph)
	{
		// "glyphBounds" is the bounds of the glyph transformed
		// by the x y location of the glyph along the base line,
		// it is therefor yet "untransformed".
		const agg::rect_i& r = glyph->bounds;
		BRect glyphBounds(r.x1 + x, r.y1 + y - 1, r.x2 + x + 1,
			r.y2 + y + 1);
			// NOTE: "-1"/"+ 1" converts the glyph bounding box from pixel
			// indices to pixel area coordinates

		if (glyph->data_type != glyph_data_outline) {
			// we cannot use the transformation pipeline
			// for vector glyphs
			double transformedX = x + fTransformOffset.x;
			double transformedY = y + fTransformOffset.y;
			entry->InitAdaptors(glyph, transformedX, transformedY,
				fMonoAdaptor, fGray8Adaptor, fPathAdaptor);
			glyphBounds.OffsetBy(fTransformOffset);
		} else {
			entry->InitAdaptors(glyph, x, y,
				fMonoAdaptor, fGray8Adaptor, fPathAdaptor);
			glyphBounds = fTransform.TransformBounds(glyphBounds);
		}

		if (!fConstrainRect.Intersects(glyphBounds))
			return;

		switch (glyph->data_type) {
//			case agg::glyph_data_mono:
//				agg::render_scanlines(fMonoAdaptor, 
//									  fMonoScanline, 
//									  binRenderer);
//				break;
//
			case agg::glyph_data_gray8:
				agg::render_scanlines(fGray8Adaptor, 
									  fGray8Scanline, 
									  fRenderer);
				break;

			case agg::glyph_data_outline: {
				agg::conv_transform<PathAdaptor, AffineTransform>
					transformedGlyph(fPathAdaptor, fTransform);
				agg::conv_curve<
					agg::conv_transform<PathAdaptor,
										AffineTransform> >
					curvedGlyph(transformedGlyph);
				curvedGlyph.approximation_scale(fScale);

				fRasterizer.add_path(curvedGlyph);
				break;
			}
			default:
				break;
		}
	}

	void Finish()
	{
		if (NeedsOutline())
			agg::render_scanlines(fRasterizer, fScanline, fRenderer);
	}

	bool NeedsOutline() const
	{
		if (fTransform.IsTranslationOnly()) {
			BPoint test(B_ORIGIN);
			fTransform.Transform(&test);
			if (roundf(test.x) == test.x
				&& roundf(test.y) == test.y) {
				return false;
			}
		}
		return true;
	}

private:
	FontCacheEntry::GlyphPathAdapter&	fPathAdaptor;
	FontCacheEntry::GlyphGray8Adapter&	fGray8Adaptor;
	FontCacheEntry::GlyphGray8Scanline&	fGray8Scanline;
	FontCacheEntry::GlyphMonoAdapter&	fMonoAdaptor;
	FontCacheEntry::GlyphMonoScanline&	fMonoScanline;

	Rasterizer&			fRasterizer;
	Scanline&			fScanline;
	Renderer&			fRenderer;

	BRect				fConstrainRect;
	AffineTransform		fTransform;
	float				fScale;
	BPoint				fTransformOffset;
};

// #pragma mark -

// _LineOffset
float
TextBlockRenderer::_LineOffset() const
{
	return fLineOffset * fFont.Size();
}

// _Update
void
TextBlockRenderer::_Update()
{
	const char* string = fText.String();
	if (!string) {
		fGlyphCount = 0;
		return;
	}

	uint32 length = strlen(string);
	if (length == 0) {
		fGlyphCount = 0;
		return;
	}

	if (fGlyphCount == 0) {
		if (_PrepareUnicodeBuffer(string, length, &fGlyphCount) < B_OK) {
			fGlyphCount = 0;
			return;
		}
	}

	// init fBounds and layout glyphs
	fNeedsLayout = true;

	Updater updater;
	_LayoutGlyphs(updater);

	fBounds = updater.Bounds();
}

// _PrepareUnicodeBuffer
status_t
TextBlockRenderer::_PrepareUnicodeBuffer(const char* utf8String,
										 uint32 length, uint32* glyphCount)
{
	int32 srcLength = length;
	int32 dstLength = srcLength * 4;

	// take care of adjusting buffer size
	if (dstLength > fUnicodeBufferSize) {
		fUnicodeBufferSize = dstLength;
		fUnicodeBuffer = (char*)realloc((void*)fUnicodeBuffer, fUnicodeBufferSize);
	}

	status_t ret;
	if (!fUnicodeBuffer) {
		ret = B_NO_MEMORY;
	} else {
		int32 state = 0;
		ret = convert_from_utf8(B_UNICODE_CONVERSION, 
								utf8String, &srcLength,
								fUnicodeBuffer, &dstLength,
								&state, B_SUBSTITUTE);
	}

	if (ret >= B_OK) {
		*glyphCount = (uint32)(dstLength / 2);
		ret = swap_data(B_INT16_TYPE, fUnicodeBuffer, dstLength,
						B_SWAP_BENDIAN_TO_HOST);
	} else {
		*glyphCount = 0;
		fprintf(stderr, "TextBlockRenderer::_PrepareUnicodeBuffer() - UTF8 -> Unicode conversion failed: %s\n", strerror(ret));
	}

	return ret;
}

#define TIMING 0

// _LayoutifNeeded
void
TextBlockRenderer::_LayoutIfNeeded()
{
	// do the initial layout if we have not done it yet
	if (fGlyphCount == 0 || fNeedsLayout)
		_Update();
}

class FontCacheReference {
public:
	FontCacheReference(FontCache* fontCache, const Font& font,
			bool needsOutline)
		: fFontCache(fontCache)
		, fWriteLocked(false)
	{
		fEntry = fFontCache->FontCacheEntryFor(font, needsOutline);
		if (fEntry && !fEntry->ReadLock()) {
			fFontCache->Recycle(fEntry);
			fEntry = NULL;
		}
	}

	~FontCacheReference()
	{
		if (!fEntry)
			return;

		if (fWriteLocked)
			fEntry->WriteUnlock();
		else
			fEntry->ReadUnlock();
	
		fFontCache->Recycle(fEntry);
	}

	FontCacheEntry* Entry() const
	{
		return fEntry;
	}

	bool WriteLock()
	{
		if (!fEntry)
			return false;

		fEntry->ReadUnlock();
		if (!fEntry->WriteLock()) {
			fFontCache->Recycle(fEntry);
			fEntry = NULL;
			return false;
		}
		fWriteLocked = true;
		return true;
	}

	FontCache*		fFontCache;
	FontCacheEntry*	fEntry;
	bool			fWriteLocked;
};

// _LayoutGlyphs
template<class GlyphConsumer>
void
TextBlockRenderer::_LayoutGlyphs(GlyphConsumer& consumer)
{
#if TIMING
bigtime_t now = system_time();
bool printLayout = false;
#endif

	if (fGlyphCount == 0) {
//printf("  updating\n");
		_Update();
	}
	if (fGlyphCount == 0 || !fUnicodeBuffer) {
//printf("  no init\n");
		return;
	}

	FontCacheReference entryReference(fFontCache, fFont,
		consumer.NeedsOutline());

	FontCacheEntry* entry = entryReference.Entry();
	if (!entry)
		return;

	if (!entry->HasGlyphs((uint16*)fUnicodeBuffer, fGlyphCount)
		&& !entryReference.WriteLock())
		return;

//printf("_LayoutGlyphs()\n");
//printf("  glyph count: %ld\n", fGlyphCount);

	if (fNeedsLayout) {
		// the cached layout is invalid
#if TIMING
printLayout = true;
#endif

		// free previous lines
		fLines.MakeEmpty();

		uint16* p = (uint16*)fUnicodeBuffer;
		uint16 lastGlyphIndex = 0;

		uint16 lastIndex = 0;
		double x0 = fFont.Hinting() ?
			floorf(fParagraphInset + 0.5) : fParagraphInset;
		double x  = x0;
		double y0 = fFont.Size() * 1.5;
		double y  = fFont.Hinting() ? floorf(y0 + 0.5) : y0;
			// TODO: y is superflous and vertical text is not supported

		double advanceX = 0.0;
		double advanceY = 0.0;
		word* currentWord = NULL;
		line* currentLine = new (nothrow) line(y, 0);

		if (!currentLine || !fLines.AddItem(currentLine)) {
			delete currentLine;
			return;
		}

		for (uint32 i = 0; i < fGlyphCount; i++) {

			// separate words
			if (*p == '\n') {
				// line break
				if (currentWord) {
					// this means the last glyphs was not a ' '
					// -> so we add the width of that glyph
					if (!currentWord->add(lastIndex, advanceX))
						return;
					if (!currentLine->add(currentWord)) {
						delete currentWord;
						return;
					}
					currentWord = NULL;
				}
				y0 += _LineOffset() * fParagraphSpacing;
				x = x0;
				y = fFont.Hinting() ? floorf(y0 + 0.5) : y0;
				advanceX = 0.0;
				advanceY = 0.0;
				++p;
				currentLine->terminated = true;
				// start a new line
				currentLine = new (nothrow) line(y, i + 1);
				if (!currentLine || !fLines.AddItem(currentLine)) {
					delete currentLine;
					return;
				}
				continue;
			}

			const GlyphCache* glyph = entry->Glyph(*p);
			if (glyph) {
				// calculate the advance offset
				if (fFont.Kerning()) {
					// TODO: lastGlyphIndex might not be initialized
					// TODO: only when within a word
					entry->GetKerning(lastGlyphIndex, *p, &advanceX, &advanceY);
					lastGlyphIndex = *p;
				}

				double totalAdvanceX = fGlyphSpacing * advanceX;
				if (advanceX > 0.0 && fGlyphSpacing > 1.0)
					totalAdvanceX += (fGlyphSpacing - 1.0) * fFont.Size();

				if (fFont.Hinting())
					totalAdvanceX = floorf(totalAdvanceX + 0.5);

				x += totalAdvanceX;
				y += advanceY;

				if (*p == ' ') {
					if (currentWord) {
						// means the last glyph was not a ' '
						// -> so we add the width of that glyph
						if (!currentWord->add(lastIndex, totalAdvanceX))
							return;
						// test for soft wrapping
						if (fBlockWidth > 0.0
							&& currentWord->end() > fBlockWidth
							&& currentLine->logical_count > 0) {

							y0 += _LineOffset();
							x = currentWord->length;
							y = fFont.Hinting() ? floorf(y0 + 0.5) : y0;
							currentWord->x_offset = 0.0;
							currentLine = new (nothrow) line(y,
								currentWord->start_glyph_index);
							if (!currentLine || !fLines.AddItem(currentLine)) {
								delete currentLine;
								return;
							}
						}
						if (!currentLine->add(currentWord)) {
							delete currentWord;
							return;
						}
						currentWord = NULL;
					}
				} else {
					if (!currentWord) {
						// means the last glyph was a ' '
						// or this is the first glyph of a new line
						currentWord = new (nothrow) word(x, i);
						if (!currentWord)
							return;
					} else {
						// means the last glyph was not a ' '
						// -> so we add the width of that glyph
						if (!currentWord->add(lastIndex, totalAdvanceX))
							return;
						// test for soft wrapping
						if (fBlockWidth > 0.0
							&& currentWord->end() > fBlockWidth
							&& currentLine->logical_count > 0) {

							y0 += _LineOffset();
							x = currentWord->length;
							y = fFont.Hinting() ? floorf(y0 + 0.5) : y0;
							currentWord->x_offset = 0.0;
							currentLine = new (nothrow) line(y,
								currentWord->start_glyph_index);
							if (!currentLine || !fLines.AddItem(currentLine)) {
								delete currentLine;
								return;
							}
						}
					}
				}
				// increment pen position
				advanceX = glyph->advance_x;
				advanceY = glyph->advance_y;
			}
			lastIndex = *p;
			++p;
		}
		if (currentWord) {
			// means the last glyph was not a ' '
			// -> so we add the width of that glyph
			if (!currentWord->add(lastIndex, advanceX))
				return;
			// test for soft wrapping
			if (fBlockWidth > 0.0 && currentWord->end() > fBlockWidth &&
				currentLine->logical_count > 0) {
				y0 += _LineOffset();
				x = currentWord->length;
				y = fFont.Hinting() ? floorf(y0 + 0.5) : y0;
				currentWord->x_offset = 0.0;
				currentLine = new (nothrow) line(y, currentWord->start_glyph_index);
				if (!currentLine || !fLines.AddItem(currentLine)) {
					delete currentLine;
					return;
				}
			}
			if (!currentLine->add(currentWord)) {
				delete currentWord;
				return;
			}
		}
		// the last line is always terminated
		currentLine->terminated = true;

		// post process lines for "center", "right" and "justify" alignment
		if (fAlignment != ALIGN_BEGIN) {
			for (int32 i = 0; line* l = fLines.ItemAt(i); i++) {
				l->justify(fAlignment, fBlockWidth, fFont.Hinting());
			}
		}
//for (int32 i = 0; line* l = fLines.ItemAt(i); i++) {
//	printf("line %ld - (y = %.2f):\n", i, l->y_offset);
//	l->print_to_stream();
//}
		fNeedsLayout = false;
	}

//printf("  layouting glyphs\n");

#if TIMING
bigtime_t layout = system_time() - now;
#endif

	// process glyphs with cached layout

	// lines
	for (int32 i = 0; line* l = fLines.ItemAt(i); i++) {
		double y = l->y_offset;
		double x = 0.0;
		// words
		for (int32 j = 0; j < l->logical_count; j++) {
			word* currentWord = l->words[j];
			// offset of first glyph in word
			x = currentWord->x_offset;
			// glyphs
			for (int32 c = 0; c < currentWord->logical_count; c++) {
				// retrieve glyph with cached index
				const GlyphCache* glyph = entry->Glyph(
					currentWord->indices[c]);
				if (glyph) {
					consumer.AddGlyph(entry, x, y, glyph);

					// advance to next glyph
					x += currentWord->offsets[c];
				}
			}
		}
	}

	consumer.Finish();

#if TIMING
bool needsOutline = consumer.NeedsOutline();
if (printLayout)
printf("layout: %lldµs, rendering: %lldµs (outline: %d)\n",
	layout, (system_time() - now) - layout, needsOutline);
else
printf("rendering: %lldµs (outline: %d)\n",
	(system_time() - now) - layout, needsOutline);
#endif
}

// _FindLineAtOffset
int32
TextBlockRenderer::_FindLineAtOffset(float offset) const
{
	// binary search for the line with the given vertical offset
	int32 lower = 0;
	int32 upper = fLines.CountItems();
	while (lower < upper) {
		int32 mid = (lower + upper) / 2;
		float midOffset = fLines.ItemAtFast(mid)->y_offset;
		if (offset <= midOffset)
			upper = mid;
		else
			lower = mid + 1;
	}
	// the offset is within the height of this line
	line* currentLine = fLines.ItemAt(lower);
	if (!currentLine || currentLine->y_offset > offset)
		lower--;
	return lower;
}

// _FindLineContaining
int32
TextBlockRenderer::_FindLineContaining(int32 glyphIndex) const
{
	// binary search for the line with the word that contains this index
	int32 lower = 0;
	int32 upper = fLines.CountItems();
	while (lower < upper) {
		int32 mid = (lower + upper) / 2;
		int32 midIndex = fLines.ItemAtFast(mid)->start_glyph_index;
		if (glyphIndex <= midIndex)
			upper = mid;
		else
			lower = mid + 1;
	}
	// the word with the glyph index is one this line
	line* currentLine = fLines.ItemAt(lower);
	if (!currentLine || currentLine->start_glyph_index > glyphIndex)
		lower--;
	return lower;
}

// _GetGlyphPositionInfo
void
TextBlockRenderer::_GetGlyphPositionInfo(int32 glyphIndex,
	BPoint* _position, int32* _lineIndex, int32* _wordIndex)
{
	*_position = B_ORIGIN;
	*_lineIndex = -1;
	*_wordIndex = -1;

//printf("glyph index: %ld\n", glyphIndex);
	_LayoutIfNeeded();

	int32 lineIndex = _FindLineContaining(glyphIndex);
	line* currentLine = fLines.ItemAt(lineIndex);
	if (!currentLine)
		return;

	*_lineIndex = lineIndex;
	_position->y = currentLine->y_offset;

	// binary search for the word that contains this index
	int32 lower = 0;
	int32 upper = currentLine->logical_count;
	if (upper <= 0)
		return;

	while (lower < upper) {
		int32 mid = (lower + upper) / 2;
		int32 midIndex = currentLine->words[mid]->start_glyph_index;
		if (glyphIndex <= midIndex)
			upper = mid;
		else
			lower = mid + 1;
	}

	word* currentWord = NULL;
	if (lower < currentLine->logical_count)
		currentWord = currentLine->words[lower];
	else
		currentWord = currentLine->words[currentLine->logical_count - 1];

	*_wordIndex = lower;

	// currentWord is guaranteed to be != NULL

	if (currentWord->start_glyph_index == glyphIndex) {
//printf("  found word %ld (beginning)\n", lower);
//currentWord->print_to_stream();
		_position->x = currentWord->x_offset;
		return;
	}

	// there is a word before currentWord, the glyph index is between those
	word* nextWord = currentWord;
//printf("  found next word %ld\n", lower);
	lower = max_c(0, lower - 1);
	*_wordIndex = lower;
	currentWord = currentLine->words[lower];
//printf("  found word %ld\n", lower);
//currentWord->print_to_stream();
	_position->x = currentWord->x_offset;
	int32 currentGlyphIndex = currentWord->start_glyph_index;

	for (int32 i = 0; i < currentWord->logical_count; i++) {
		_position->x += currentWord->offsets[i];
		currentGlyphIndex++;
		if (currentGlyphIndex == glyphIndex)
			return;
	}

	if (nextWord != currentWord) {
//printf("between words\n");
		// if we got this far, we are inbetween the current and next word,
		// figure out the width of a backspace
		float width = nextWord->x_offset - _position->x;
		int32 numSpaces = nextWord->start_glyph_index - currentGlyphIndex;
		for (int32 i = 0; i < numSpaces; i++) {
			_position->x += width / numSpaces;
			currentGlyphIndex++;
			if (currentGlyphIndex == glyphIndex)
				return;
		}
printf("should not be here (between words)\n");
	} else {
		// we are behind the last word of the line
//printf("behind last word\n");
		if (fSpaceWidth == 0.0)
			fSpaceWidth = GlyphWidth((uint16)' ');
		while (currentGlyphIndex < glyphIndex) {
			_position->x += fSpaceWidth;
			currentGlyphIndex++;
		}
		return;
	}

printf("TextBlockRenderer::_GetGlyphPositionInfo() - should not be here\n");
}

// _GlyphIndexAt
int32
TextBlockRenderer::_GlyphIndexAt(TextBlockRenderer::line* currentLine,
	BPoint position)
{
	// binary search for the word that contains the position
	int32 lower = 0;
	int32 upper = currentLine->logical_count;
	if (upper <= 0)
		return 0;

	while (lower < upper) {
		int32 mid = (lower + upper) / 2;
		float midPosition = currentLine->words[mid]->x_offset;
		if (position.x <= midPosition)
			upper = mid;
		else
			lower = mid + 1;
	}

	word* currentWord = NULL;
	if (lower < currentLine->logical_count)
		currentWord = currentLine->words[lower];
	else
		currentWord = currentLine->words[currentLine->logical_count - 1];

	// currentWord is guaranteed to be != NULL

	if (currentWord->x_offset == position.x) {
//printf("  found word %ld (beginning)\n", lower);
//currentWord->print_to_stream();
		return currentWord->start_glyph_index;
	}

	// there is a word before currentWord, the position is between those
	word* nextWord = currentWord;
//printf("  found next word %ld\n", lower);
	lower = max_c(0, lower - 1);
	currentWord = currentLine->words[lower];
	int32 glyphIndex = currentWord->start_glyph_index;
//printf("  found word %ld\n", lower);
//currentWord->print_to_stream();
	float currentGlyphOffset = currentWord->x_offset;

	for (int32 i = 0; i < currentWord->logical_count; i++) {
		currentGlyphOffset += currentWord->offsets[i];
		if (currentGlyphOffset > position.x)
			return glyphIndex;
		glyphIndex++;
	}

	if (nextWord != currentWord) {
//printf("between words\n");
		// if we got this far, we are inbetween the current and next word,
		// figure out the width of a backspace
		float width = nextWord->x_offset - currentGlyphOffset;
		int32 numSpaces = nextWord->start_glyph_index - glyphIndex;
		for (int32 i = 0; i < numSpaces; i++) {
			currentGlyphOffset += width / numSpaces;
			if (currentGlyphOffset > position.x)
				return glyphIndex;
			glyphIndex++;
		}
printf("should not be here (between words)\n");
	} else {
		// we are behind the last word of the line
//printf("behind last word\n");
		if (fSpaceWidth == 0.0)
			fSpaceWidth = GlyphWidth((uint16)' ');
		while (currentGlyphOffset < position.x) {
			currentGlyphOffset += fSpaceWidth;
			glyphIndex++;
		}
		return glyphIndex;
	}

	return glyphIndex;
}

// RenderText
void
TextBlockRenderer::RenderText(Painter& painter)
{
	if (painter.PixelFormat() == YCbCr422
		|| painter.PixelFormat() == YCbCr444) {

		// TODO: Find a better way to do this! The problem
		// is that the global alpha (implemented in terms of a fake 
		// rasterizer gamma function) is ignored for direct rendering
		// of cached glyph bitmaps. The temporary solution is to modify
		// the renderer color directly in order to encode the global
		// alpha in the renderer color. But the result is a bit different
		// from the outline glyph rendering, so there must be a bug
		// or wrong assumption somewhere.
		renderer_type_ycc& r = painter.RendererYCC();
		agg::ycbcra8 color = r.color();
		uint8 oldAlpha = color.a;
		color.a = (uint8)(oldAlpha * painter.GlobalAlpha() / 255.0);
		r.color(color);

		GlyphRenderer<rasterizer_type,
					  scanline_unpacked_type,
					  renderer_type_ycc>
			renderer(fPathAdaptor, fGray8Adaptor, fGray8Scanline,
					 fMonoAdaptor, fMonoScanline,
					 painter.Rasterizer(),
					 painter.Scanline(), painter.RendererYCC(),
					 painter.Bounds(), painter.Transformation(),
					 painter.Scale());
	
		_LayoutGlyphs(renderer);

		color.a = oldAlpha;
		r.color(color);
	} else {
		// TODO: See above.
		renderer_type& r = painter.RendererRGB();
		agg::rgba8 color = r.color();
		uint8 oldAlpha = color.a;
		color.a = (uint8)(oldAlpha * painter.GlobalAlpha() / 255.0);
		r.color(color);

		GlyphRenderer<rasterizer_type,
					  scanline_unpacked_type,
					  renderer_type>
			renderer(fPathAdaptor, fGray8Adaptor, fGray8Scanline,
					 fMonoAdaptor, fMonoScanline,
					 painter.Rasterizer(),
					 painter.Scanline(), painter.RendererRGB(),
					 painter.Bounds(), painter.Transformation(),
					 painter.Scale());
	
		_LayoutGlyphs(renderer);

		color.a = oldAlpha;
		r.color(color);
	}
}

