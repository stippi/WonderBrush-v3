/*
 * TextLayout.h
 *
 *  Created on: 23.08.2012
 *      Author: stippi
 */

#ifndef TEXTLAYOUT_H_
#define TEXTLAYOUT_H_

#include <jni.h>

#include "AggContext.h"
#include "Font.h"


struct StyleRun {
	int							start;

	Font						font;

	double						ascent;		// Not yet used
	double						descent;	// Not yet used

	int							fgRed;
	int							fgGreen;
	int							fgBlue;

	int							bgRed;
	int							bgGreen;
	int							bgBlue;

	bool						strikeOut;
	int							strikeRed;
	int							strikeGreen;
	int							strikeBlue;

	bool						underline;
	unsigned					underlineStyle;
	int							underlineRed;
	int							underlineGreen;
	int							underlineBlue;

	// TODO: Probably needs metrics as well to define custom gaps in the text
	// where the client can paint images that are wrapped like ordinary glyphs.
};


struct GlyphInfo {
	unsigned					charCode;

	const agg::glyph_cache*		glyph;

	double						x;
	double						y;

	double						maxAscend;
	double						maxDescend;

	unsigned					lineIndex;

	StyleRun*					styleRun;
};


struct LineInfo {
	unsigned					startOffset;
	double						y;
	double						height;
	double						maxAscent;
	double						maxDescent;
};


static const unsigned MOVEMENT_CHAR				= 1 << 0;
static const unsigned MOVEMENT_CLUSTER			= 1 << 1;
static const unsigned MOVEMENT_WORD				= 1 << 2;
static const unsigned MOVEMENT_WORD_END			= 1 << 3;
static const unsigned MOVEMENT_WORD_START		= 1 << 4;


static const unsigned ALIGNMENT_LEFT			= 0;
static const unsigned ALIGNMENT_RIGHT			= 1;
static const unsigned ALIGNMENT_CENTER			= 2;

static const unsigned SELECTION_FULL			= 1 << 16;
static const unsigned SELECTION_LINE_DELIMITER	= 1 << 17;
static const unsigned SELECTION_LAST_LINE		= 1 << 20;

class TextLayout {
public:
	TextLayout(const AggContext* context);
	virtual ~TextLayout();

	void setText(const char* text);
	void setFirstLineInset(double inset);
	void setLineInset(double inset);
	void setWidth(double width);
	void setAlignment(unsigned alignment);
	void setJustify(bool justify);
	void setGlyphSpacing(double spacing);
	void setLineSpacing(double spacing);
	void setTabs(jdouble* tabs, unsigned count);

	void clearStyleRuns();
	bool addStyleRun(int start, const char* fontPath,
		double fontSize, unsigned fontStyle,
		int fgRed, int fgGreen, int fgBlue,
		int bgRed, int bgGreen, int bgBlue,
		bool strikeOut, int strikeRed, int strikeGreen, int strikeBlue,
		bool underline, unsigned underlineStyle,
		int underlineRed, int underlineGreen, int underlineBlue);

	void layout();

	inline unsigned getGlyphCount() const
	{
		return fGlyphInfoCount;
	}

	inline void getInfo(int index, const agg::glyph_cache** glyph, double* x,
		double* y, double* height, Color& fgColor, bool& strikeOut,
		Color& strikeColor, bool& underline, unsigned& underlineStyle,
		Color& underlineColor) const
	{
		*glyph = fGlyphInfoBuffer[index].glyph;
		*x = fGlyphInfoBuffer[index].x;
		*y = fGlyphInfoBuffer[index].y;
		*height = fGlyphInfoBuffer[index].maxAscend
			+ fGlyphInfoBuffer[index].maxDescend;

		StyleRun* style = fGlyphInfoBuffer[index].styleRun;
		if (style != NULL) {
			*height = style->font.getSize();
			if (style->fgRed >= 0) {
				fgColor.r = style->fgRed;
				fgColor.g = style->fgGreen;
				fgColor.b = style->fgBlue;
			}
			if (style->strikeOut && style->strikeRed >= 0) {
				strikeOut = true;
				strikeColor.r = style->strikeRed;
				strikeColor.g = style->strikeGreen;
				strikeColor.b = style->strikeBlue;
				strikeColor.a = 255;
			}
			if (style->underline && style->underlineRed >= 0) {
				underline = true;
				underlineStyle = style->underlineStyle;
				underlineColor.r = style->underlineRed;
				underlineColor.g = style->underlineGreen;
				underlineColor.b = style->underlineBlue;
				underlineColor.a = 255;
			}
		}
	}

	inline void getInfo(unsigned index, unsigned* lineIndex,
		double* x, double* advanceX, double* lineTop, double* lineBottom,
		Color& bgColor) const
	{
		*lineIndex = fGlyphInfoBuffer[index].lineIndex;

		*x = fGlyphInfoBuffer[index].x;
		if (fGlyphInfoBuffer[index].glyph != NULL)
			*advanceX = fGlyphInfoBuffer[index].glyph->advance_x;
		else
			*advanceX = 3.0;
		*lineTop = fLineInfoBuffer[*lineIndex].y;
		*lineBottom = *lineTop + fLineInfoBuffer[*lineIndex].height;

		StyleRun* style = fGlyphInfoBuffer[index].styleRun;
		if (style != NULL) {
			if (style->bgRed >= 0) {
				bgColor.r = style->bgRed;
				bgColor.g = style->bgGreen;
				bgColor.b = style->bgBlue;
			}
		}
	}

	inline void getGlyphBoundingBox(unsigned index, double* x1, double* y1,
		double* x2, double* y2) const
	{
		double y = fGlyphInfoBuffer[index].y;

		*x1 = fGlyphInfoBuffer[index].x;
		*y1 = y - fGlyphInfoBuffer[index].maxAscend;
		*x2 = *x1;
		*y2 = y + fGlyphInfoBuffer[index].maxDescend;

		if (index < fGlyphInfoCount - 1
			&& fGlyphInfoBuffer[index + 1].lineIndex
				== fGlyphInfoBuffer[index].lineIndex) {
			*x2 = fGlyphInfoBuffer[index + 1].x;
		} else if (fGlyphInfoBuffer[index].glyph != NULL) {
			*x2 += fGlyphInfoBuffer[index].glyph->advance_x;
		}
	}

	inline double getFirstLineInset() const
	{
		return fFirstLineInset;
	}

	inline double getWidth()
	{
		if (fWidth > 0)
			return fWidth;
		else
			return getActualWidth();
	}

	double getActualWidth();
	double getHeight();

	double getScaleX() const;

	int getLineCount();
	int getLineIndex(int textOffset);
	double getLineWidth(int lineIndex);
	void getLineBounds(int lineIndex, double* x1, double* y1,
		double* x2, double* y2);

	int getLineOffsets(jint offsets[], unsigned count);

	unsigned getOffset(double x, double y, bool& rightOfCenter);

	void getLineMetrics(int lineIndex, double buffer[]);

	int getPreviousOffset(int offset, unsigned movement);
	int getNextOffset(int offset, unsigned movement);

private:
	bool init(const char* text, FontEngine& fontEngine,
		FontManager& fontManager, bool hinting, double scaleX,
		unsigned subpixelScale);

	void layout(FontEngine& fontEngine, FontManager& fontManager,
		bool kerning, double scaleX, unsigned subpixelScale);
	void applyAlignment(const double width);

	void invalidateLayout();
	void validateLayout();

	bool appendGlyph(unsigned charCode, const agg::glyph_cache* glyph,
		StyleRun* styleRun);
	bool appendLine(unsigned startOffset, double y, double lineHeight,
		double maxAscent, double maxDescent);

private:
	const AggContext*	fContext;

	double				fFirstLineInset;
	double				fLineInset;
	double				fWidth;
	unsigned			fAlignment;
	bool				fJustify;

	double				fHeight;

	double				fGlyphSpacing;
	double				fLineSpacing;

	GlyphInfo*			fGlyphInfoBuffer;
	unsigned			fGlyphInfoBufferSize;
	unsigned			fGlyphInfoCount;

	LineInfo*			fLineInfoBuffer;
	unsigned			fLineInfoBufferSize;
	unsigned			fLineInfoCount;

	StyleRun*			fStyleRunBuffer;
	unsigned			fStyleRunBufferSize;
	unsigned			fStyleRunCount;

	double*				fTabBuffer;
	unsigned			fTabCount;

	bool				fLayoutPerformed;
};

#endif /* TEXTLAYOUT_H_ */