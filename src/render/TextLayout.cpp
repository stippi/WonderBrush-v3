/*
 * Copyright 2012 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 *
 * Parts of the code:
 *
 * Copyright 2001-2009, Haiku Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#include "TextLayout.h"

#include <algorithm>
#include <string.h>

#include "FontCache.h"
#include "UTF8Utils.h"


enum {
	CHAR_CLASS_DEFAULT,
	CHAR_CLASS_WHITESPACE,
	CHAR_CLASS_GRAPHICAL,
	CHAR_CLASS_QUOTE,
	CHAR_CLASS_PUNCTUATION,
	CHAR_CLASS_PARENS_OPEN,
	CHAR_CLASS_PARENS_CLOSE,
	CHAR_CLASS_END_OF_TEXT
};


static unsigned
getCharClassification(unsigned charCode)
{
	// TODO: Should check against a list of characters containing also
	// word breakers from other languages.

	switch (charCode) {
		case '\0':
			return CHAR_CLASS_END_OF_TEXT;

		case ' ':
		case '\t':
		case '\n':
			return CHAR_CLASS_WHITESPACE;

		case '=':
		case '+':
		case '@':
		case '#':
		case '$':
		case '%':
		case '^':
		case '&':
		case '*':
		case '\\':
		case '|':
		case '<':
		case '>':
		case '/':
		case '~':
			return CHAR_CLASS_GRAPHICAL;

		case '\'':
		case '"':
			return CHAR_CLASS_QUOTE;

		case ',':
		case '.':
		case '?':
		case '!':
		case ';':
		case ':':
		case '-':
			return CHAR_CLASS_PUNCTUATION;

		case '(':
		case '[':
		case '{':
			return CHAR_CLASS_PARENS_OPEN;

		case ')':
		case ']':
		case '}':
			return CHAR_CLASS_PARENS_CLOSE;

		default:
			return CHAR_CLASS_DEFAULT;
	}
}


static inline bool
canEndLine(GlyphInfo* buffer, int offset, int count)
{
	if (offset == count - 1)
		return true;

	if (offset < 0 || offset > count)
		return false;

	unsigned charCode = buffer[offset].charCode;
	unsigned classification = getCharClassification(charCode);

	// wrapping is always allowed at end of text and at newlines
	if (classification == CHAR_CLASS_END_OF_TEXT || charCode == '\n')
		return true;

	unsigned nextCharCode = buffer[offset + 1].charCode;
	unsigned nextClassification = getCharClassification(nextCharCode);

	// never separate a punctuation char from its preceding word
	if (classification == CHAR_CLASS_DEFAULT
		&& nextClassification == CHAR_CLASS_PUNCTUATION) {
		return false;
	}

	if ((classification == CHAR_CLASS_WHITESPACE
			&& nextClassification != CHAR_CLASS_WHITESPACE)
		|| (classification != CHAR_CLASS_WHITESPACE
			&& nextClassification == CHAR_CLASS_WHITESPACE)) {
		return true;
	}

	// allow wrapping after whitespace, unless more whitespace (except for
	// newline) follows
	if (classification == CHAR_CLASS_WHITESPACE
		&& (nextClassification != CHAR_CLASS_WHITESPACE
			|| nextCharCode == '\n')) {
		return true;
	}

	// allow wrapping after punctuation chars, unless more punctuation, closing
	// parenthesis or quotes follow
	if (classification == CHAR_CLASS_PUNCTUATION
		&& nextClassification != CHAR_CLASS_PUNCTUATION
		&& nextClassification != CHAR_CLASS_PARENS_CLOSE
		&& nextClassification != CHAR_CLASS_QUOTE) {
		return true;
	}

	// allow wrapping after quotes, graphical chars and closing parenthesis only
	// if whitespace follows (not perfect, but seems to do the right thing most
	// of the time)
	if ((classification == CHAR_CLASS_QUOTE
			|| classification == CHAR_CLASS_GRAPHICAL
			|| classification == CHAR_CLASS_PARENS_CLOSE)
		&& nextClassification == CHAR_CLASS_WHITESPACE) {
		return true;
	}

	return false;
}


TextLayout::TextLayout(FontCache* fontCache)
	:
	fFontCache(fontCache),
	
	fFont("DejaVuSans.ttf", 12.0),
	fAscent(0.0),
	fDescent(0.0),
	
	fFirstLineInset(0.0),
	fLineInset(0.0),
	fWidth(0.0),
	fAlignment(ALIGNMENT_LEFT),
	fJustify(false),

	fHeight(0.0),

	fGlyphSpacing(0.0),			// -0.2-0.20, Default: 0.0
	fLineSpacing(0.0),

	fGlyphInfoBuffer(NULL),
	fGlyphInfoBufferSize(0),
	fGlyphInfoCount(0),

	fLineInfoBuffer(NULL),
	fLineInfoBufferSize(0),
	fLineInfoCount(0),

	fStyleRunBuffer(NULL),
	fStyleRunBufferSize(0),
	fStyleRunCount(0),

	fTabBuffer(NULL),
	fTabCount(0),
	
	fSubpixelRendering(false),
	fKerning(true),
	fHinting(true),

	fLayoutPerformed(false)
{
}


TextLayout::TextLayout(const TextLayout& other)
	:
	fFont(other.fFont),
	fGlyphInfoBuffer(NULL),
	fLineInfoBuffer(NULL),
	fStyleRunBuffer(NULL),
	fTabBuffer(NULL)
{
	*this = other;
}


TextLayout&
TextLayout::operator=(const TextLayout& other)
{
	fFontCache = other.fFontCache;
	
	fFont = other.fFont;
	fAscent = other.fAscent;
	fDescent = other.fDescent;

	fFirstLineInset = other.fFirstLineInset;
	fLineInset = other.fLineInset;
	fWidth = other.fWidth;
	fAlignment = other.fAlignment;
	fJustify = other.fJustify;

	fHeight = other.fHeight;

	fGlyphSpacing = other.fGlyphSpacing;
	fLineSpacing = other.fLineSpacing;

	fGlyphInfoBuffer = (GlyphInfo*)realloc(fGlyphInfoBuffer,
		other.fGlyphInfoBufferSize * sizeof(GlyphInfo));
	fGlyphInfoBufferSize = other.fGlyphInfoBufferSize;
	fGlyphInfoCount = other.fGlyphInfoCount;
	if (fGlyphInfoCount > 0) {
		memcpy(fGlyphInfoBuffer, other.fGlyphInfoBuffer,
			fGlyphInfoCount * sizeof(GlyphInfo));
	}

	fLineInfoBuffer = (LineInfo*)realloc(fLineInfoBuffer,
		other.fLineInfoBufferSize * sizeof(LineInfo));
	fLineInfoBufferSize = other.fLineInfoBufferSize;
	fLineInfoCount = other.fLineInfoCount;
	if (fLineInfoCount > 0) {
		memcpy(fLineInfoBuffer, other.fLineInfoBuffer,
			fLineInfoCount * sizeof(LineInfo));
	}

	fStyleRunBuffer = (StyleRun*)realloc(fStyleRunBuffer,
		other.fStyleRunBufferSize * sizeof(StyleRun));
	fStyleRunBufferSize = other.fStyleRunBufferSize;
	fStyleRunCount = other.fStyleRunCount;
	if (fStyleRunCount > 0) {
		memcpy(fStyleRunBuffer, other.fStyleRunBuffer,
			fStyleRunCount * sizeof(StyleRun));
	}

	fTabBuffer = (double*)realloc(fTabBuffer,
		other.fTabCount * sizeof(double));;
	fTabCount = other.fTabCount;
	
	fSubpixelRendering = other.fSubpixelRendering;
	fKerning = other.fKerning;
	fHinting = other.fHinting;

	fLayoutPerformed = other.fLayoutPerformed;
	
	return *this;
}


TextLayout::~TextLayout()
{
	free(fGlyphInfoBuffer);
	free(fLineInfoBuffer);
	free(fStyleRunBuffer);
	free(fTabBuffer);
}


void
TextLayout::setText(const char* text)
{
	unsigned subpixelScale = fSubpixelRendering ? 3 : 1;
	init(text, fFontCache->getFontEngine(), fFontCache->getFontManager(),
		fHinting, TextRenderer::AUTO_HINT_SCALE, subpixelScale);

	invalidateLayout();
}


void
TextLayout::setFont(const Font& font)
{
	if (fFont != font) {
		fFont = font;
		invalidateLayout();
	}
}


void
TextLayout::setFirstLineInset(double inset)
{
	if (fFirstLineInset != inset) {
		fFirstLineInset = inset;
		invalidateLayout();
	}
}


void
TextLayout::setLineInset(double inset)
{
	if (fLineInset != inset) {
		fLineInset = inset;
		invalidateLayout();
	}
}


void
TextLayout::setWidth(double width)
{
	if (fWidth != width) {
		fWidth = width;
		invalidateLayout();
	}
}


void
TextLayout::setAlignment(unsigned alignment)
{
	if (alignment > ALIGNMENT_CENTER)
		return;
	if (fAlignment != alignment) {
		fAlignment = alignment;
		invalidateLayout();
	}
}


void
TextLayout::setJustify(bool justify)
{
	if (fJustify != justify) {
		fJustify = justify;
		invalidateLayout();
	}
}


void
TextLayout::setGlyphSpacing(double spacing)
{
	if (fGlyphSpacing != spacing) {
		fGlyphSpacing = spacing;
		invalidateLayout();
	}
}


void
TextLayout::setLineSpacing(double spacing)
{
	if (fLineSpacing != spacing) {
		fLineSpacing = spacing;
		invalidateLayout();
	}
}


void
TextLayout::setTabs(double* tabs, unsigned count)
{
	if (fTabCount == 0 && count == 0)
		return;

	if (fTabCount != count) {
		free(fTabBuffer);
		fTabBuffer = NULL;
		fTabCount = 0;
		if (count > 0) {
			fTabBuffer = (double*)malloc(count * sizeof(double));
			if (fTabBuffer == NULL)
				return;
			fTabCount = count;
		}
	}

	for (unsigned i = 0; i < count; i++)
		fTabBuffer[i] = tabs[i];

	invalidateLayout();
}


void
TextLayout::clearStyleRuns()
{
	fStyleRunCount = 0;
}


bool
TextLayout::addStyleRun(int start, const char* fontPath,
	double fontSize, unsigned fontStyle,
	double metricsAscent, double metricsDescent, double metricsWidth,
	int fgRed, int fgGreen, int fgBlue,
	int bgRed, int bgGreen, int bgBlue,
	bool strikeOut, int strikeRed, int strikeGreen, int strikeBlue,
	bool underline, unsigned underlineStyle,
	int underlineRed, int underlineGreen, int underlineBlue)
{
//printf("TextLayout::addStyleRun(%d, font('%s', %.1f, %u), "
//	"color(%d, %d, %d)) (index: %u)\n",
//	start, fontPath, fontSize, fontStyle, fgRed, fgGreen, fgBlue,
//	fStyleRunCount);
	// Enlarge buffer if necessary
	if (fStyleRunCount == fStyleRunBufferSize) {
		int size = fStyleRunBufferSize + 64;

		StyleRun* buffer = (StyleRun*) realloc(fStyleRunBuffer,
			size * sizeof(StyleRun));
		if (buffer == NULL)
			return false;

		fStyleRunBufferSize = size;
		fStyleRunBuffer = buffer;
	}

//	printf("adding style: %d, %s, %.1f, fg(%d, %d, %d), bg(%d, %d, %d)\n",
//		start, fontPath, fontSize, fgRed, fgGreen, fgBlue, bgRed, bgGreen,
//		bgBlue);
//	fflush(stdout);

	// Store given information
	fStyleRunBuffer[fStyleRunCount].start = start;

	new (&(fStyleRunBuffer[fStyleRunCount].font)) Font(fontPath, fontSize,
		fontStyle);

	fStyleRunBuffer[fStyleRunCount].ascent = metricsAscent;
	fStyleRunBuffer[fStyleRunCount].descent = metricsDescent;
	fStyleRunBuffer[fStyleRunCount].width = metricsWidth;

	fStyleRunBuffer[fStyleRunCount].fgRed = fgRed;
	fStyleRunBuffer[fStyleRunCount].fgGreen = fgGreen;
	fStyleRunBuffer[fStyleRunCount].fgBlue = fgBlue;

	fStyleRunBuffer[fStyleRunCount].bgRed = bgRed;
	fStyleRunBuffer[fStyleRunCount].bgGreen = bgGreen;
	fStyleRunBuffer[fStyleRunCount].bgBlue = bgBlue;

	fStyleRunBuffer[fStyleRunCount].strikeOut = strikeOut;
	fStyleRunBuffer[fStyleRunCount].strikeRed = strikeRed;
	fStyleRunBuffer[fStyleRunCount].strikeGreen = strikeGreen;
	fStyleRunBuffer[fStyleRunCount].strikeBlue = strikeBlue;

	fStyleRunBuffer[fStyleRunCount].underline = underline;
	fStyleRunBuffer[fStyleRunCount].underlineStyle = underlineStyle;
	fStyleRunBuffer[fStyleRunCount].underlineRed = underlineRed;
	fStyleRunBuffer[fStyleRunCount].underlineGreen = underlineGreen;
	fStyleRunBuffer[fStyleRunCount].underlineBlue = underlineBlue;

	fStyleRunCount++;

	return true;
}


void
TextLayout::layout()
{
	if (fGlyphInfoCount == 0)
		return;

	unsigned subpixelScale = fSubpixelRendering ? 3 : 1;
	layout(fFontCache->getFontEngine(), fFontCache->getFontManager(),
		fKerning, TextRenderer::AUTO_HINT_SCALE, subpixelScale);
}


double
TextLayout::getActualWidth()
{
	validateLayout();

	double maxWidth = 0.0;

	double scale = getScaleX();

	for (unsigned i = 0; i < fGlyphInfoCount; i++) {
		double width = (fGlyphInfoBuffer[i].x + fGlyphInfoBuffer[i].advanceX)
			/ scale;

		if (width > maxWidth)
			maxWidth = width;
	}
	return maxWidth;
}


double
TextLayout::getHeight()
{
	validateLayout();

	if (fLineInfoCount > 0) {
		return fLineInfoBuffer[fLineInfoCount - 1].y
			+ fLineInfoBuffer[fLineInfoCount - 1].height;
	}

	if (fStyleRunCount > 0) {
		return fStyleRunBuffer[0].font.getSize();
	}

	return fFont.getSize();
}


double
TextLayout::getScaleX() const
{
	unsigned subpixelScale = fSubpixelRendering ? 3 : 1;
	return TextRenderer::AUTO_HINT_SCALE * subpixelScale;
}


int
TextLayout::getLineCount()
{
	validateLayout();
	if (fGlyphInfoCount > 0)
		return fGlyphInfoBuffer[fGlyphInfoCount - 1].lineIndex + 1;
	return 1;
}


int
TextLayout::getLineIndex(int textOffset)
{
	validateLayout();
	if (textOffset <= 0)
		return 0;

	if (textOffset >= (int) fGlyphInfoCount) {
		if (fLineInfoCount > 0)
			return fLineInfoCount - 1;
		else
			return 0;
	}

	return fGlyphInfoBuffer[textOffset].lineIndex;
}


double
TextLayout::getLineWidth(int lineIndex)
{
	validateLayout();
	double width = 0.0;

	for (unsigned i = 0; i < fGlyphInfoCount; i++) {
		if ((int) fGlyphInfoBuffer[i].lineIndex < lineIndex)
			continue;
		else if ((int) fGlyphInfoBuffer[i].lineIndex > lineIndex)
			break;

		width = fGlyphInfoBuffer[i].x + fGlyphInfoBuffer[i].advanceX;
	}

	double scale = getScaleX();
	width /= scale;

	return width;
}


void
TextLayout::getLineBounds(int lineIndex, double* x1, double* y1,
	double* x2, double* y2)
{
	validateLayout();

	*x1 = 0.0;
	*x2 = 0.0;
	*y1 = 0.0;
	*y2 = 0.0;

	if (fStyleRunCount > 0)
		*y2 = fStyleRunBuffer[0].font.getSize();

	bool foundLineStart = false;

	for (unsigned i = 0; i < fGlyphInfoCount; i++) {
		if ((int) fGlyphInfoBuffer[i].lineIndex < lineIndex)
			continue;
		else if ((int) fGlyphInfoBuffer[i].lineIndex > lineIndex)
			break;

		if (!foundLineStart) {
			foundLineStart = true;
			getGlyphBoundingBox(i, x1, y1, x2, y2);
		}

		*x2 = fGlyphInfoBuffer[i].x + fGlyphInfoBuffer[i].advanceX;
	}

	double scale = getScaleX();

	*x1 /= scale;
	*x2 /= scale;

//	printf("getLineBounds: %.1f, %.1f -> %.1f, %.1f\n",
//		*x1, *y1, *x2, *y2);
//	fflush(stdout);
}


int
TextLayout::getLineOffsets(int offsets[], unsigned count)
{
	validateLayout();

	if (fLineInfoCount == 0 && count > 0) {
		offsets[0] = 0;
		return 1;
	}

	unsigned i = 0;
	for (; i < fLineInfoCount && i < count; i++)
		offsets[i] = fLineInfoBuffer[i].startOffset;

	return i;
}


unsigned
TextLayout::getOffset(double x, double y, bool& rightOfCenter)
{
	validateLayout();

	double scale = getScaleX();
	x *= scale;

	rightOfCenter = false;

	if (fGlyphInfoCount == 0 || fLineInfoCount == 0
		|| fLineInfoBuffer[0].y > y) {
		// Above first line or empty text
//		printf("getOffset(%.4f, %.4f) - above first line\n", x / scale, y);
//		fflush(stdout);
		return 0;
	}

	unsigned lineIndex = 0;

	if (floor(fLineInfoBuffer[fLineInfoCount - 1].y
		+ fLineInfoBuffer[fLineInfoCount - 1].height + 0.5) > y) {
		// TODO: Optimize, can binary search line here:
		for (; lineIndex < fLineInfoCount; lineIndex++) {
			double lineBottom = floor(fLineInfoBuffer[lineIndex].y
				+ fLineInfoBuffer[lineIndex].height + 0.5);
			if (lineBottom > y)
				break;
		}
	} else {
		lineIndex = fLineInfoCount - 1;
	}

	// Found line
	unsigned offset = fLineInfoBuffer[lineIndex].startOffset;
	unsigned end;
	if (lineIndex < fLineInfoCount - 1)
		end = fLineInfoBuffer[lineIndex + 1].startOffset - 1;
	else
		end = fGlyphInfoCount - 1;

//	printf("hit testing line %u (y: %.4f, line: %.4f->%.4f)\n", lineIndex, y,
//		fLineInfoBuffer[lineIndex].y,
//		fLineInfoBuffer[lineIndex].y + fLineInfoBuffer[lineIndex].height);
//	fflush(stdout);
//	char glyph[2];
//	glyph[1] = 0;

	// TODO: Optimize, can binary search offset here:
	for (; offset <= end; offset++) {
//		glyph[0] = (char) fGlyphInfoBuffer[offset].charCode;
//		printf("  testing %u (%s)", offset, glyph);
		double x1 = fGlyphInfoBuffer[offset].x;
		if (x1 > x) {
//			printf("  -> x1 > x\n");
//			fflush(stdout);
			return offset;
		}

		// x2 is the location at the right bounding box of the glyph
		double x2 = x1;
		if (fGlyphInfoBuffer[offset].glyph != NULL)
			x2 += fGlyphInfoBuffer[offset].glyph->advance_x;

		// x3 is the location of the next glyph, which may be different from
		// x2 in case the line is justified.
		double x3;
		if (offset < end - 1)
			x3 = fGlyphInfoBuffer[offset + 1].x;
		else
			x3 = x2;

		if (x3 > x) {
//			printf("  -> x2 > x\n");
//			fflush(stdout);
			rightOfCenter = x > (x1 + x2) / 2.0;
			return offset;
		}
//		printf("\n");
	}

	rightOfCenter = true;
	return end;
}


void
TextLayout::getLineMetrics(int lineIndex, double buffer[])
{
	validateLayout();

	if (lineIndex < 0 || lineIndex > (int) fLineInfoCount)
		return;

	// buffer: ascent, descent, averageCharWidth, leading, height

	if (lineIndex == (int) fLineInfoCount || fGlyphInfoCount == 0) {
		if (fStyleRunCount > 0) {
			buffer[0] = fStyleRunBuffer[0].ascent;
			buffer[1] = fStyleRunBuffer[0].descent;
			buffer[2] = fStyleRunBuffer[0].width;
			buffer[4] = fStyleRunBuffer[0].font.getSize();
			buffer[3] = buffer[4] - (buffer[0] + buffer[1]);
//			printf("getLineMetrics() %.1f, %.1f, %.1f, %.1f, %.1f (empty)\n",
//				buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
//			fflush(stdout);
		} else {
			buffer[0] = 0.0;
			buffer[1] = 0.0;
			buffer[2] = 0.0;
			buffer[3] = 0.0;
			buffer[4] = 0.0;
		}
		return;
	}

	buffer[0] = fLineInfoBuffer[lineIndex].maxAscent;
	buffer[1] = fLineInfoBuffer[lineIndex].maxDescent;

	// Figure out average char width
	double scale = getScaleX();

	double charWidthSum = 0.0;
	unsigned charCount = 0;
	for (unsigned i = fLineInfoBuffer[lineIndex].startOffset;
		i < fGlyphInfoCount; i++) {

		if ((int) fGlyphInfoBuffer[i].lineIndex != lineIndex)
			break;
		if (fGlyphInfoBuffer[i].glyph != NULL) {
			charWidthSum += fGlyphInfoBuffer[i].glyph->advance_x;
			charCount++;
		}
	}
	buffer[2] = (charWidthSum / scale) / charCount;

	buffer[4] = fLineInfoBuffer[lineIndex].height;
	buffer[3] = buffer[4] - (buffer[0] + buffer[1]);

//	printf("getLineMetrics() %.1f, %.1f, %.1f, %.1f, %.1f\n",
//		buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
//	fflush(stdout);
}


int
TextLayout::getPreviousOffset(int offset, unsigned movement)
{
	if (offset <= 0 || fGlyphInfoCount == 0)
		return 0;

	if (offset >= (int) fGlyphInfoCount)
		return fGlyphInfoCount - 1;

	int startOffset = offset;

	while (true) {
		unsigned rightClassification
			= getCharClassification(fGlyphInfoBuffer[offset].charCode);

		offset--;
		if (offset == 0)
			return offset;

		unsigned leftClassification
			= getCharClassification(fGlyphInfoBuffer[offset].charCode);

		if (leftClassification != CHAR_CLASS_DEFAULT
			&& rightClassification != CHAR_CLASS_DEFAULT)
			return offset;

		switch (movement) {
		case MOVEMENT_CHAR:
		case MOVEMENT_CLUSTER:
			return offset;

		case MOVEMENT_WORD:
			// TODO: Should be platform + direction dependent
			if (rightClassification == CHAR_CLASS_DEFAULT
				&& leftClassification != CHAR_CLASS_DEFAULT
				&& offset + 1 < startOffset) {
				return offset + 1;
			} else if (rightClassification != CHAR_CLASS_DEFAULT
				&& leftClassification == CHAR_CLASS_DEFAULT
				&& offset + 1 < startOffset) {
				return offset + 1;
			}
			break;
		case MOVEMENT_WORD_START:
			if (rightClassification == CHAR_CLASS_DEFAULT
				&& leftClassification != CHAR_CLASS_DEFAULT
				&& offset + 1 < startOffset) {
				return offset + 1;
			}
			break;
		case MOVEMENT_WORD_END:
			if (rightClassification != CHAR_CLASS_DEFAULT
				&& leftClassification == CHAR_CLASS_DEFAULT
				&& offset + 1 < startOffset) {
				return offset + 1;
			}
			break;
		}
	}
}


int
TextLayout::getNextOffset(int offset, unsigned movement)
{
	if (offset < 0 || fGlyphInfoCount == 0)
		return 0;

	if (offset >= (int) fGlyphInfoCount)
		return fGlyphInfoCount - 1;

	while (true) {
		unsigned leftClassification
			= getCharClassification(fGlyphInfoBuffer[offset].charCode);

		offset++;
		if (offset == (int) fGlyphInfoCount - 1)
			return offset;

		unsigned rightClassification
			= getCharClassification(fGlyphInfoBuffer[offset].charCode);

		if (leftClassification != CHAR_CLASS_DEFAULT
			&& rightClassification != CHAR_CLASS_DEFAULT)
			return offset;

		switch (movement) {
		case MOVEMENT_CHAR:
		case MOVEMENT_CLUSTER:
			return offset;

		case MOVEMENT_WORD:
			// TODO: Should be platform + direction dependent
			if (leftClassification != CHAR_CLASS_DEFAULT
				&& rightClassification == CHAR_CLASS_DEFAULT) {
				return offset;
			} else if (leftClassification == CHAR_CLASS_DEFAULT
				&& rightClassification != CHAR_CLASS_DEFAULT) {
				return offset;
			}
			break;
		case MOVEMENT_WORD_START:
			if (leftClassification != CHAR_CLASS_DEFAULT
				&& rightClassification == CHAR_CLASS_DEFAULT) {
				return offset;
			}
			break;
		case MOVEMENT_WORD_END:
			if (leftClassification == CHAR_CLASS_DEFAULT
				&& rightClassification != CHAR_CLASS_DEFAULT) {
				return offset;
			}
			break;
		}
	}
}


void
TextLayout::getTextBounds(int textOffset, double& x1, double& y1,
	double& x2, double& y2)
{
	validateLayout();
	
	if (textOffset < 0) {
		x1 = 0.0;
		y1 = 0.0;
		x2 = 0.0;
		y2 = 0.0;
	} else if (textOffset >= (int) fGlyphInfoCount) {
		if (fGlyphInfoCount == 0) {
			getLineBounds(0, &x1, &y1, &x2, &y2);
		} else {
			getGlyphBoundingBox(fGlyphInfoCount - 1, &x1, &y1, &x2, &y2);
			x1 = x2;

			const double scale = getScaleX();
			x1 /= scale;
			x2 /= scale;
		}
	} else {
		getGlyphBoundingBox(textOffset, &x1, &y1, &x2, &y2);

		const double scale = getScaleX();
		x1 /= scale;
		x2 /= scale;
	}
}


bool
TextLayout::init(const char* text, FontEngine& fontEngine,
	FontManager& fontManager, bool hinting, double scaleX,
	unsigned subpixelScale)
{
	AutoWriteLocker _(FontCache::getInstance());
	
	fGlyphInfoCount = 0;
	fLineInfoCount = 0;

	BString resolvedFontPath = FontCache::getInstance()
		->resolveFont(fFont.getName());
    double height = fFont.getSize();

	if (!fontEngine.load_font(resolvedFontPath.String(), 0,
		agg::glyph_ren_outline, height * scaleX * subpixelScale, height)) {
		fprintf(stderr, "Error loading font: '%s'\n",
			resolvedFontPath.String());
	}

	fAscent = fontEngine.ascender();
	fDescent = fontEngine.descender();

    fontEngine.hinting(hinting);

	const char* p = text;

	int styleIndex = -1;
	unsigned offset = 0;

	while (true) {
		unsigned charCode = UTF8ToCharCode(&p);
		if (charCode == '\0')
			break;

		if (styleIndex < ((int) fStyleRunCount) - 1) {
			StyleRun* nextStyleRun = &(fStyleRunBuffer[styleIndex + 1]);
			if (nextStyleRun->start == (int) offset) {
				height = nextStyleRun->font.getSize();

				resolvedFontPath = FontCache::getInstance()
					->resolveFont(nextStyleRun->font.getName());

				if (!fontEngine.load_font(resolvedFontPath.String(), 0,
					agg::glyph_ren_outline, height * scaleX * subpixelScale,
					height)) {
					fprintf(stderr, "Error loading font: '%s'\n",
						resolvedFontPath.String());
				}

				// Init these two after having loaded the font in the engine.
				// But only do so if the StyleRun does not provide it's own
				// metrics.
				if (nextStyleRun->width == 0.0) {
					nextStyleRun->ascent = fontEngine.ascender();
					nextStyleRun->descent = -fontEngine.descender();
				}

				styleIndex++;
			}
		}
		StyleRun* styleRun = NULL;
		if (styleIndex >= 0 && styleIndex < (int) fStyleRunCount)
			styleRun = &(fStyleRunBuffer[styleIndex]);

		const agg::glyph_cache* glyph = NULL;
		if (charCode != '\n' && charCode != '\t') {
			glyph = fontManager.glyph(charCode);
//			if (glyph != NULL) {
//				char t[2];
//				t[0] = (char) charCode;
//				t[1] = 0;
//				printf("[%s] advance: %.1f\n", t, glyph->advance_x);
//				fflush(stdout);
//			}
		}

		if (!appendGlyph(charCode, glyph, styleRun))
			return false;

		offset++;
	}

	return true;
}


void
TextLayout::layout(FontEngine& fontEngine, FontManager& fontManager,
	bool kerning, double scaleX, unsigned subpixelScale)
{
	fLineInfoCount = 0;

	if (fGlyphInfoCount == 0)
		return;

	AutoWriteLocker _(FontCache::getInstance());

	const double width = fWidth * scaleX * subpixelScale;
	const double additionalGlyphSpacing = fGlyphSpacing * scaleX
		* subpixelScale;

	double x = fFirstLineInset * scaleX * subpixelScale;
	double y = 0;

	unsigned lineIndex = 0;
	unsigned lineStart = 0;

	StyleRun* lastLoadedStyleRun = NULL;

	for (unsigned i = 0; i < fGlyphInfoCount; i++) {
		const agg::glyph_cache* glyph = fGlyphInfoBuffer[i].glyph;

		unsigned charClassification = getCharClassification(
			fGlyphInfoBuffer[i].charCode);

		double advanceX = 0.0;
		double advanceY = 0.0;

		// increment position
		if (fGlyphInfoBuffer[i].styleRun != NULL
			&& fGlyphInfoBuffer[i].styleRun->width > 0.0) {
			// Use the metrics provided by the StyleRun
			advanceX = fGlyphInfoBuffer[i].styleRun->width * scaleX
				* subpixelScale + additionalGlyphSpacing;
		} else if (glyph != NULL) {
			advanceX = glyph->advance_x + additionalGlyphSpacing;
			advanceY = glyph->advance_y;
		}

		bool nextLine = false;
		bool lineBreak = false;

		if (fGlyphInfoBuffer[i].charCode == '\t') {
			// Figure out tab width, it's the width between the last two tab
			// stops.
			double tabWidth = 0.0;
			if (fTabCount > 0)
				tabWidth = fTabBuffer[fTabCount - 1];
			if (fTabCount > 1)
				tabWidth -= fTabBuffer[fTabCount - 2];
			tabWidth *= scaleX * subpixelScale;

			// Try to find a tab stop that is farther than the current x
			// offset
			double tabOffset = 0.0;
			for (unsigned tabIndex = 0; tabIndex < fTabCount; tabIndex++) {
				tabOffset = fTabBuffer[tabIndex] * scaleX * subpixelScale;
				if (tabOffset > x)
					break;
			}

			// If no tab stop has been found, make the tab stop a multiple of
			// the tab width
			if (tabOffset <= x && tabWidth > 0.0)
				tabOffset = ((int) (x / tabWidth) + 1) * tabWidth;

			if (tabOffset - x > 0.0)
				advanceX = tabOffset - x;
		}

		fGlyphInfoBuffer[i].advanceX = advanceX;

		if (fGlyphInfoBuffer[i].charCode == '\n') {
			nextLine = true;
			lineBreak = true;
			fGlyphInfoBuffer[i].x = x;
			fGlyphInfoBuffer[i].y = y;
		} else if (width > 0 && x + advanceX > width) {
			if (charClassification == CHAR_CLASS_WHITESPACE) {
				advanceX = 0.0;
			} else if (i > lineStart) {
				nextLine = true;
				// The current glyph extends outside the width, we need to wrap
				// to the next line. See if the previous offset can be the end
				// of the line.
				int lineEnd = i - 1;
				while (lineEnd > (int) lineStart
					&& !canEndLine(fGlyphInfoBuffer, lineEnd,
						fGlyphInfoCount)) {
					lineEnd--;
				}

				if (lineEnd > (int) lineStart) {
					// Found a place to perform a line break.
					i = lineEnd + 1;
					// Adjust the glyph info to point at the changed buffer
					// position
					glyph = fGlyphInfoBuffer[i].glyph;
					if (fGlyphInfoBuffer[i].styleRun != NULL
						&& fGlyphInfoBuffer[i].styleRun->width > 0.0) {
						advanceX = fGlyphInfoBuffer[i].styleRun->width
							+ additionalGlyphSpacing;
						advanceY = 0.0;
					} else if (glyph != NULL) {
						advanceX = glyph->advance_x + additionalGlyphSpacing;
						advanceY = glyph->advance_y;
					} else {
						advanceX = 0.0;
						advanceY = 0.0;
					}
				} else {
					// Just break where we are.
				}
			}
		}

		double lineHeight = 0.0;
		if (nextLine) {
			// * Initialize the max ascent/descent of all preceding glyph infos
			// on the current/last line
			// * Adjust the baseline offset according to the max ascent
			// * Fill in the line index.
			unsigned lineEnd;
			if (lineBreak)
				lineEnd = i;
			else
				lineEnd = i - 1;

			double maxAscent = 0.0;
			double maxDescent = 0.0;

			for (unsigned j = lineStart; j <= lineEnd; j++) {
				if (fGlyphInfoBuffer[j].styleRun != NULL) {
					if (fGlyphInfoBuffer[j].styleRun->font.getSize()
							> lineHeight) {
						lineHeight
							= fGlyphInfoBuffer[j].styleRun->font.getSize();
					}
					if (fGlyphInfoBuffer[j].styleRun->ascent > maxAscent)
						maxAscent = fGlyphInfoBuffer[j].styleRun->ascent;
					if (fGlyphInfoBuffer[j].styleRun->descent > maxDescent)
						maxDescent = fGlyphInfoBuffer[j].styleRun->descent;
				} else {
					if (fFont.getSize() > lineHeight)
						lineHeight = fFont.getSize();
					if (fAscent > maxAscent)
						maxAscent = fAscent;
					if (fDescent > maxDescent)
						maxDescent = fDescent;
				}
			}

			if (!appendLine(lineStart, y, lineHeight, maxAscent, maxDescent))
				return;

			for (unsigned j = lineStart; j <= lineEnd; j++) {
				fGlyphInfoBuffer[j].maxAscend = maxAscent;
				fGlyphInfoBuffer[j].maxDescend = maxDescent;
				fGlyphInfoBuffer[j].lineIndex = lineIndex;
				fGlyphInfoBuffer[j].y += maxAscent;
			}
		}

		if (nextLine) {
			// Start position of the next line
			x = fLineInset * scaleX * subpixelScale;
			y += lineHeight + fLineSpacing;

			if (lineBreak)
				lineStart = i + 1;
			else
				lineStart = i;

			lineIndex++;
		} else {
			if (i < fGlyphInfoCount && kerning && i > lineStart) {
				if (glyph != NULL && fGlyphInfoBuffer[i - 1].glyph != NULL
					&& ((fGlyphInfoBuffer[i].styleRun != NULL
							&& fGlyphInfoBuffer[i - 1].styleRun != NULL
							&& fGlyphInfoBuffer[i].styleRun->font
								== fGlyphInfoBuffer[i - 1].styleRun->font)
						|| (fGlyphInfoBuffer[i].styleRun == NULL
							&& fGlyphInfoBuffer[i - 1].styleRun == NULL))) {

					// For kerning to work at this point, the engine needs
					// to have the right font loaded
					if (lastLoadedStyleRun != fGlyphInfoBuffer[i].styleRun) {
						lastLoadedStyleRun = fGlyphInfoBuffer[i].styleRun;
						double size = lastLoadedStyleRun->font.getSize();

						BString resolvedFontPath = FontCache::getInstance()
							->resolveFont(lastLoadedStyleRun->font.getName());

						if (!fontEngine.load_font(resolvedFontPath.String(), 0,
							agg::glyph_ren_outline,
							size * scaleX * subpixelScale, size)) {
							fprintf(stderr, "Error loading font: '%s'\n",
								resolvedFontPath.String());
						}
					}

					fontEngine.add_kerning(
		            	fGlyphInfoBuffer[i - 1].glyph->glyph_index,
		            	glyph->glyph_index, &x, &y
		            );
				}
			}
		}

		if (!lineBreak && i < fGlyphInfoCount) {
			fGlyphInfoBuffer[i].x = x;
			fGlyphInfoBuffer[i].y = y;
		}

		x += advanceX;
		y += advanceY;
	}

	// The last line may not have been appended and initialized yet.
	if (lineStart <= fGlyphInfoCount - 1) {
		double lineHeight = 0.0;
		double maxAscent = 0.0;
		double maxDescent = 0.0;

		for (unsigned j = lineStart; j <= fGlyphInfoCount - 1; j++) {
			if (fGlyphInfoBuffer[j].styleRun != NULL) {
				if (fGlyphInfoBuffer[j].styleRun->font.getSize() > lineHeight)
					lineHeight = fGlyphInfoBuffer[j].styleRun->font.getSize();
				if (fGlyphInfoBuffer[j].styleRun->ascent > maxAscent)
					maxAscent = fGlyphInfoBuffer[j].styleRun->ascent;
				if (fGlyphInfoBuffer[j].styleRun->descent > maxDescent)
					maxDescent = fGlyphInfoBuffer[j].styleRun->descent;
			} else {
				if (fFont.getSize() > lineHeight)
					lineHeight = fFont.getSize();
				if (fAscent > maxAscent)
					maxAscent = fAscent;
				if (fDescent > maxDescent)
					maxDescent = fDescent;
			}
		}

		if (!appendLine(lineStart, y, lineHeight, maxAscent, maxDescent))
			return;

		for (unsigned j = lineStart; j <= fGlyphInfoCount - 1; j++) {
			fGlyphInfoBuffer[j].maxAscend = maxAscent;
			fGlyphInfoBuffer[j].maxDescend = maxDescent;
			fGlyphInfoBuffer[j].lineIndex = lineIndex;
			fGlyphInfoBuffer[j].y += maxAscent;
		}
	}

//	// Dump the line layout for debugging purposes:
//	printf("Line count: %u\n", fLineInfoCount);
//	char glyph[2];
//	glyph[1] = 0;
//	for (unsigned i = 0; i < fLineInfoCount; i++) {
//		glyph[0]
//			= fGlyphInfoBuffer[fLineInfoBuffer[i].startOffset].charCode & 0xff;
//		if (glyph[0] == '\n')
//			glyph[0] = '_';
//		printf("  [%u] start: %u (%s) y: %.1f\n", i,
//			fLineInfoBuffer[i].startOffset, glyph, fLineInfoBuffer[i].y);
//	}
//	fflush(stdout);

	applyAlignment(width);

	fLayoutPerformed = true;
}


void
TextLayout::applyAlignment(const double width)
{
	if (fAlignment == ALIGNMENT_LEFT && !fJustify)
		return;

	if (fGlyphInfoCount == 0)
		return;

	int lineIndex = -1;
	double spaceLeft = 0.0;
	double charSpace = 0.0;
	double whiteSpace = 0.0;
	bool seenChar = false;

	// Iterate all glyphs backwards. On the last character of the next line,
	// the position of the character determines the available space to be
	// distributed (spaceLeft).
	for (int i = fGlyphInfoCount - 1; i >= 0; i--) {
		if ((int) fGlyphInfoBuffer[i].lineIndex != lineIndex) {
			bool lineBreak = fGlyphInfoBuffer[i].charCode == '\n'
				|| i == (int) fGlyphInfoCount - 1;
			lineIndex = fGlyphInfoBuffer[i].lineIndex;

			// The position of the last character determines the available
			// space.
			spaceLeft = width - fGlyphInfoBuffer[i].x;

			// If the character is visible, the width of the character needs to
			// be subtracted from the available space, otherwise it would be
			// pushed outside the line.
			if (fGlyphInfoBuffer[i].glyph != NULL
				&& fGlyphInfoBuffer[i].glyph->bounds.x2
					> fGlyphInfoBuffer[i].glyph->bounds.x1) {
				spaceLeft -= fGlyphInfoBuffer[i].glyph->advance_x;
			}

			charSpace = 0.0;
			whiteSpace = 0.0;
			seenChar = false;

			if (lineBreak || !fJustify) {
				if (fAlignment == ALIGNMENT_CENTER)
					spaceLeft /= 2.0;
				else if (fAlignment == ALIGNMENT_LEFT)
					spaceLeft = 0.0;
			} else {
				// Figure out how much chars and white space chars are on the
				// line. Don't count trailing white space.
				unsigned charCount = 0;
				unsigned spaceCount = 0;
				for (int j = i; j >= 0; j--) {
					if ((int) fGlyphInfoBuffer[j].lineIndex != lineIndex) {
						j++;
						break;
					}
					unsigned classification = getCharClassification(
						fGlyphInfoBuffer[j].charCode);
					if (classification == CHAR_CLASS_WHITESPACE) {
						if (charCount > 0)
							spaceCount++;
						else if (j < i)
							spaceLeft += fGlyphInfoBuffer[i].advanceX;
					} else {
						charCount++;
					}
				}

				// The first char is not shifted when justifying, so it doesn't
				// contribute.
				if (charCount > 0)
					charCount--;

				// Check if it looks better if both whitespace and chars get
				// some space distributed, in case there are only 1 or two
				// space chars on the line.
				double spaceLeftForSpace = spaceLeft;
				double spaceLeftForChars = spaceLeft;

				if (spaceCount > 0) {
					double spaceCharRatio = (double) spaceCount / charCount;
					if (spaceCount < 3 && spaceCharRatio < 0.4) {
						spaceLeftForSpace = spaceLeft * 2.0 * spaceCharRatio;
						spaceLeftForChars = spaceLeft - spaceLeftForSpace;
					} else
						spaceLeftForChars = 0.0;
				}

				if (spaceCount > 0)
					whiteSpace = spaceLeftForSpace / spaceCount;
				if (charCount > 0)
					charSpace = spaceLeftForChars / charCount;
			}
		}

		// Each character is pushed towards the right by the space that is
		// still available. When justification is performed, the shift is
		// gradually decreased. This works since the iteration is backwards
		// and the characters on the right are pushed farthest.
		fGlyphInfoBuffer[i].x += spaceLeft;
		if (i < (int) fGlyphInfoCount - 1
			&& (int) fGlyphInfoBuffer[i + 1].lineIndex == lineIndex) {
			fGlyphInfoBuffer[i].advanceX = fGlyphInfoBuffer[i + 1].x
				- fGlyphInfoBuffer[i].x;
		}

		// The shift (spaceLeft) is reduced depending on the character
		// classification.
		unsigned classification
			= getCharClassification(fGlyphInfoBuffer[i].charCode);
		if (classification == CHAR_CLASS_WHITESPACE) {
			if (seenChar)
				spaceLeft -= whiteSpace;
		} else {
			seenChar = true;
			spaceLeft -= charSpace;
		}
	}
}


void
TextLayout::invalidateLayout()
{
	fLayoutPerformed = false;
}


void
TextLayout::validateLayout()
{
	if (!fLayoutPerformed)
		layout();
}


bool
TextLayout::appendGlyph(unsigned charCode, const agg::glyph_cache* glyph,
	StyleRun* styleRun)
{
	// Enlarge buffer if necessary
	if (fGlyphInfoCount == fGlyphInfoBufferSize) {
		int size = fGlyphInfoBufferSize + 64;

		GlyphInfo* buffer = (GlyphInfo*) realloc(fGlyphInfoBuffer,
			size * sizeof(GlyphInfo));
		if (buffer == NULL)
			return false;

		fGlyphInfoBufferSize = size;
		fGlyphInfoBuffer = buffer;
	}

	// Store given information
	fGlyphInfoBuffer[fGlyphInfoCount].charCode = charCode;
	fGlyphInfoBuffer[fGlyphInfoCount].glyph = glyph;
	fGlyphInfoBuffer[fGlyphInfoCount].x = 0;
	fGlyphInfoBuffer[fGlyphInfoCount].y = 0;
	fGlyphInfoBuffer[fGlyphInfoCount].advanceX = 0;
	fGlyphInfoBuffer[fGlyphInfoCount].maxAscend = 0;
	fGlyphInfoBuffer[fGlyphInfoCount].maxDescend = 0;
	fGlyphInfoBuffer[fGlyphInfoCount].lineIndex = 0;
	fGlyphInfoBuffer[fGlyphInfoCount].styleRun = styleRun;

	fGlyphInfoCount++;

	return true;
}


bool
TextLayout::appendLine(unsigned startOffset, double y, double lineHeight,
	double maxAscent, double maxDescent)
{
	// Enlarge buffer if necessary
	if (fLineInfoCount == fLineInfoBufferSize) {
		int size = fLineInfoBufferSize + 8;

		LineInfo* buffer = (LineInfo*) realloc(fLineInfoBuffer,
			size * sizeof(LineInfo));
		if (buffer == NULL)
			return false;

		fLineInfoBufferSize = size;
		fLineInfoBuffer = buffer;
	}

	// Store given information
	fLineInfoBuffer[fLineInfoCount].startOffset = startOffset;
	fLineInfoBuffer[fLineInfoCount].y = y;
	fLineInfoBuffer[fLineInfoCount].height = lineHeight;
	fLineInfoBuffer[fLineInfoCount].maxAscent = maxAscent;
	fLineInfoBuffer[fLineInfoCount].maxDescent = maxDescent;

	fLineInfoCount++;

	return true;
}



