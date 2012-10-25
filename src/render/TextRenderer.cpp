/*
 * Copyright 2012 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */
#include "TextRenderer.h"

#include "FontCache.h"
#include "TextLayout.h"
#include "UTF8Utils.h"


static inline bool
operator!=(const Color& a, const Color& b)
{
	return a.r != b.r || a.g != b.g || a.b != b.b || a.a != b.a;
}


TextRenderer::TextRenderer(FontCache* fontCache)
	:
	fBuffer(),

	fPixelFormat(fBuffer),
	fRenderer(fPixelFormat),
	fRendererSolid(fRenderer),

	fForeground(0, 0, 0, (255 << 8) | 255),
	fBackground((255 << 8) | 255, (255 << 8) | 255,
		(255 << 8) | 255, (255 << 8) | 255),

	fGamma(1.0 / 2.2),				// 0.50-2.50, Default: 1.0
	fPrimaryWeight(1.0 / 3.0),	// 0.00-1.00, Default: 1/3
	fGlyphWidthScale(1.0),		// 0.75-1.25, Default: 1.0
	fGlyphSpacing(0.0),			// -0.2-0.20, Default: 0.0
	fFauxWeight(0.0),			// -1.0-1.00, Default: 0.0
	fFauxItalic(0.0),			// -1.0-1.00, Default: 0.0

	fPrimaryWeights(fPrimaryWeight, 2.0 / 9.0, 1.0 / 9.0),
	fPixelFormatLCD(fBuffer, fPrimaryWeights),
	fRendererLCD(fPixelFormatLCD),
	fRendererSolidLCD(fRendererLCD),

	fScanline(),
	fRasterizer(),
	fPath(),

	fFontCache(fontCache),

	fBaseMatrix(),
	fMatrix(),
	fGlyph(getFontManager().path_adaptor()),
    fTransformedGlyph(fGlyph, fMatrix),
    fFauxWeightGlyph(fTransformedGlyph),

	fHinting(true),
	fKerning(true),
	fGrayScale(false)
{
	fRasterizer.gamma(agg::gamma_power(fGamma));
}


void
TextRenderer::attachToBuffer(unsigned char* data, int width, int height,
	int stride)
{
	fBuffer.attach(data, width, height, stride);
	fRenderer.reset_clipping(true);
	fRendererLCD.reset_clipping(true);
}


void
TextRenderer::setClipping(int x, int y, int width, int height)
{
	fRenderer.clip_box(x, y, x + width, y + height);
	fRendererLCD.clip_box(x * 3, y, (x + width) * 3, y + height);
}


void
TextRenderer::unsetClipping()
{
	fRenderer.reset_clipping(true);
	fRendererLCD.reset_clipping(true);
}


void
TextRenderer::clear()
{
	fRenderer.clear(fBackground);
}

void
TextRenderer::render()
{
	agg::render_scanlines(fRasterizer, fScanline, fRendererSolid);
	fRasterizer.reset();
}


void
TextRenderer::setForeground(int red, int green, int blue, int alpha)
{
	fForeground.r = red;
	fForeground.g = green;
	fForeground.b = blue;
	fForeground.a = alpha;
}


void
TextRenderer::setBackground(int red, int green, int blue, int alpha)
{
	fBackground.r = red;
	fBackground.g = green;
	fBackground.b = blue;
	fBackground.a = alpha;
}


FontEngine&
TextRenderer::getFontEngine() const
{
	return fFontCache->getFontEngine();
}


FontManager&
TextRenderer::getFontManager() const
{
	return fFontCache->getFontManager();
}


void
TextRenderer::setTransformation(const Transformation& transformation)
{
	fBaseMatrix = transformation;
	fGlyph.approximation_scale(fBaseMatrix.scale());
}


bool
TextRenderer::loadFont(const char* fontFilePath, double height)
{
	BString resolvedFontPath = FontCache::getInstance()->resolveFont(
		fontFilePath);

	return getFontEngine().load_font(resolvedFontPath.String(), 0,
		agg::glyph_ren_outline, height, height);
}


double
TextRenderer::drawString(const char* text, double x, double y)
{
	if (fGrayScale) {
		return drawString(fRendererSolid, text, x, y, 1);
	} else {
		return drawString(fRendererSolidLCD, text, x, y, 3);
	}
}


double
TextRenderer::drawText(TextLayout* layout, double x, double y,
	int selectionStart, int selectionEnd, const Color& fg, const Color& bg,
	unsigned flags)
{
	if (fGrayScale) {
		return drawText(fRendererSolid, layout, x, y,
			selectionStart, selectionEnd, fg, bg, flags, 1);
	} else {
		return drawText(fRendererSolidLCD, layout, x, y,
			selectionStart, selectionEnd, fg, bg, flags, 3);
	}
}


template<class RendererType>
double
TextRenderer::drawString(RendererType& renderer,
	const char* text, double x, double y, unsigned subpixelScale)
{
	FontEngine& fontEngine = getFontEngine();
	FontManager& fontManager = getFontManager();

	double scaleX = AUTO_HINT_SCALE;
	double height = fontEngine.height();

	fontEngine.width(height * scaleX * subpixelScale);
	fontEngine.hinting(fHinting);

	const char* p = text;

	x *= subpixelScale;
	double startX = x;

	// Offset baseline so that original x and y coordinates are located
	// at top-left corner of the string bounding box.
	y += floor(fontEngine.ascender() + 0.5);

	double lineHeight = fontEngine.height();

	fRasterizer.clip_box(0, 0, fBuffer.width() * subpixelScale,
		fBuffer.height());

	renderer.color(fForeground);

	while (true) {
		unsigned charCode = UTF8ToCharCode(&p);
		if (charCode == '\0')
			break;

		if (charCode == '\n') {
			x = startX;
			y += lineHeight;
			// Don't apply kerning for the previous glyph and the next one
			fontManager.reset_last_glyph();
			continue;
		}

		const agg::glyph_cache* glyph = fontManager.glyph(charCode);

		if (glyph == NULL)
			continue;

		if (fKerning) {
			fontManager.add_kerning(&x, &y);
		}

		fontManager.init_embedded_adaptors(glyph, 0, 0);

		if (glyph->data_type == agg::glyph_data_outline) {
			double ty = fHinting ? floor(y + 0.5) : y;

			fMatrix.reset();
			fMatrix *= agg::trans_affine_scaling(
				fGlyphWidthScale / scaleX, 1);
			fMatrix *= agg::trans_affine_skewing(
				fFauxItalic * subpixelScale / 3, 0);
			fMatrix *= agg::trans_affine_translation(
				startX + x / scaleX, ty);
			fMatrix *= fBaseMatrix;

			fRasterizer.reset();

			if (fabs(fFauxWeight) < 0.05) {
				fRasterizer.add_path(fTransformedGlyph);
			} else {
				fFauxWeightGlyph.weight(
					-fFauxWeight * height * subpixelScale / 15);
				fRasterizer.add_path(fFauxWeightGlyph);
			}

			agg::render_scanlines(fRasterizer, fScanline, renderer);
		}

		// increment pen position
		x += glyph->advance_x + fGlyphSpacing * scaleX * subpixelScale;
		y += glyph->advance_y;
	}

	return y;
}


template<class RendererType>
double
TextRenderer::drawText(RendererType& renderer,
	TextLayout* layout, double xOffset, double yOffset,
	int selectionStart, int selectionEnd,
	const Color& selectionFG, const Color& selectionBG, unsigned flags,
	unsigned subpixelScale)
{
	double scaleX = AUTO_HINT_SCALE;
	double xOffsetScaled = xOffset * subpixelScale;
	double scale = subpixelScale * scaleX;

	double x = layout->getFirstLineInset() * scale;
	const double width = layout->getWidth();

	fRasterizer.clip_box(0, 0, fBuffer.width() * subpixelScale,
		fBuffer.height());

	int count = layout->getGlyphCount();

//	printf("drawText(selection: %d, %d) count: %d\n", selectionStart,
//		selectionEnd, count);
//	fflush(stdout);

	unsigned lineIndex = 0;
	double advanceX = 0.0;
#if 0
	double lineTop = 0.0;
	double lineBottom = layout->getHeight();
	Color bg(fBackground);

	if (count > 0) {

		layout->getInfo(0, &lineIndex, &x, &advanceX, &lineTop, &lineBottom,
				bg);

		for (int index = 1; index < count; index++) {

			unsigned nextLineIndex;
			double nextX;
			double nextLineTop;
			double nextLineBottom;
			Color nextBG(fBackground);

			layout->getInfo(index, &nextLineIndex, &nextX, &advanceX,
				&nextLineTop, &nextLineBottom, nextBG);

			double x2 = nextX;
			if (lineIndex != nextLineIndex)
				x2 = width * scale;

			if (selectionStart >= 0 && selectionEnd >= selectionStart
				&& index - 1 >= selectionStart && index - 1 <= selectionEnd) {
				fRenderer.copy_bar(
					x / scale + xOffset, yOffset + lineTop + 0.5,
					x2 / scale + xOffset, yOffset + lineBottom - 0.5,
					selectionBG);
			} else if ((flags & TEXT_TRANSPARENT) == 0 || bg != fBackground) {
				fRenderer.copy_bar(
					x / scale + xOffset, yOffset + lineTop + 0.5,
					x2 / scale + xOffset, yOffset + lineBottom - 0.5, bg);
			}

			lineIndex = nextLineIndex;
			x = nextX;
			lineTop = nextLineTop;
			lineBottom = nextLineBottom;
			bg = nextBG;
		}

		// Fill rest of last line to fill width of layout
		if (advanceX > 0.0) {
			double x2 = x + advanceX;
			if (selectionStart >= 0 && selectionEnd >= selectionStart
				&& count - 1 >= selectionStart && count - 1 <= selectionEnd) {
				fRenderer.copy_bar(
					x / scale + xOffset, yOffset + lineTop + 0.5,
					x2 / scale + xOffset, yOffset + lineBottom - 0.5,
					selectionBG);
			} else if ((flags & TEXT_TRANSPARENT) == 0 || bg != fBackground) {
				fRenderer.copy_bar(
					x / scale + xOffset, yOffset + lineTop + 0.5,
					x2 / scale + xOffset, yOffset + lineBottom - 0.5, bg);
			}
		}
	}

	if ((flags & SELECTION_LAST_LINE) != 0) {
		x += advanceX;
		advanceX = 5 * scale;
		double x2 = x + advanceX;
		fRenderer.copy_bar(
			x / scale + xOffset, yOffset + lineTop + 0.5,
			x2 / scale + xOffset, yOffset + lineBottom - 0.5,
			selectionBG);
	}

	if ((flags & TEXT_TRANSPARENT) == 0 && x + advanceX < width * scale) {
		double x2 = x + advanceX;
		fRenderer.copy_bar(
			x2 / scale + xOffset, yOffset + lineTop + 0.5,
			width + xOffset, yOffset + lineBottom - 0.5, fBackground);
	}
#endif

	x = layout->getFirstLineInset() * scale;
	double lastX = x;
	double y = fHinting ? floor(yOffset + 0.5) : yOffset;
	double lastY = y;
	double lastAdvanceX = 0.0;
	double height = 0.0;
	double lastHeight = height;
	bool lastStrikeOut = false;
	Color lastStrikeColor = fForeground;
	bool lastUnderline = false;
	Color lastUnderlineColor = fForeground;

	agg::rect_i clipRect = fRenderer.clip_box();
	
	FontManager& fontManager = getFontManager();

	for (int index = 0; index < count; index++) {

		const agg::glyph_cache* glyph;

		Color fg(fForeground);

		bool strikeOut = false;
		Color strikeColor(fg);

		bool underline = false;
		unsigned underlineStyle = 0;
		Color underlineColor(fg);

		layout->getInfo(index, &glyph, &x, &y, &height, fg, strikeOut,
			strikeColor, underline, underlineStyle, underlineColor);

		double ty = fHinting ? floor(yOffset + y + 0.5) : yOffset + y;

		if (glyph != NULL && glyph->data_type == agg::glyph_data_outline
			&& xOffset + (x + glyph->bounds.x2) / scale >= clipRect.x1
			&& xOffset + (x + glyph->bounds.x1) / scale <= clipRect.x2
			&& ty + glyph->bounds.y2 >= clipRect.y1
			&& ty + glyph->bounds.y1 <= clipRect.y2) {
			fontManager.init_embedded_adaptors(glyph, 0, 0);

			fMatrix.reset();
			fMatrix *= agg::trans_affine_scaling(
				fGlyphWidthScale / scaleX, 1);
			fMatrix *= agg::trans_affine_skewing(
				fFauxItalic * subpixelScale / 3, 0);
			fMatrix *= agg::trans_affine_translation(
				xOffsetScaled + x / scaleX, ty);
			fMatrix *= fBaseMatrix;

			fRasterizer.reset();

			if (fabs(fFauxWeight) < 0.05) {
				fRasterizer.add_path(fTransformedGlyph);
			} else {
				fFauxWeightGlyph.weight(
					-fFauxWeight * glyph->height * subpixelScale / 15);
				fRasterizer.add_path(fFauxWeightGlyph);
			}

			if (selectionStart >= 0 && selectionEnd >= selectionStart
				&& index >= selectionStart && index <= selectionEnd) {
				renderer.color(selectionFG);
			} else {
				renderer.color(fg);
			}

			agg::render_scanlines(fRasterizer, fScanline, renderer);
		}

		if (lastStrikeOut) {
			drawStrikeOut(index, xOffset, lastX, x, lastAdvanceX, scale, lastY,
				ty, lastHeight, selectionStart, selectionEnd, selectionFG,
				lastStrikeColor);
		}

		if (lastUnderline) {
			drawUnderline(index, xOffset, lastX, x, lastAdvanceX, scale, lastY,
				ty, lastHeight, selectionStart, selectionEnd, selectionFG,
				lastUnderlineColor);
		}

		lastX = x;
		lastY = ty;
		lastAdvanceX = advanceX;
		lastHeight = height;
		lastStrikeOut = strikeOut;
		lastStrikeColor = strikeColor;
		lastUnderline = underline;
		lastUnderlineColor = underlineColor;
	}

	if (lastStrikeOut) {
		drawStrikeOut(count, xOffset, lastX, x + lastAdvanceX, lastAdvanceX,
			scale, lastY, lastY, lastHeight, selectionStart, selectionEnd,
			selectionFG, lastStrikeColor);
	}

	if (lastUnderline) {
		drawUnderline(count, xOffset, lastX, x + lastAdvanceX, lastAdvanceX,
			scale, lastY, lastY, lastHeight, selectionStart, selectionEnd,
			selectionFG, lastUnderlineColor);
	}

	return yOffset + y;
}


void
TextRenderer::drawStrikeOut(int index, double xOffset, double lastX, double x,
	double lastAdvanceX, double scale, double lastY, double ty, double height,
	int selectionStart, int selectionEnd,
	const Color& selectionFG, const Color& strikeColor)
{
	double x2;
	if (lastY == ty) {
		x2 = x;
	} else {
		x2 = lastX + lastAdvanceX;
	}

	double offsetY = height / 3.5;

	if (selectionStart >= 0 && selectionEnd >= selectionStart
		&& (index - 1) >= selectionStart
		&& (index - 1) <= selectionEnd) {
		fRenderer.copy_bar(
			lastX / scale + xOffset, lastY - offsetY,
			x2 / scale + xOffset, lastY - offsetY, selectionFG);
	} else {
		fRenderer.copy_bar(
			lastX / scale + xOffset, lastY - offsetY,
			x2 / scale + xOffset, lastY - offsetY, strikeColor);
	}
}


void
TextRenderer::drawUnderline(int index, double xOffset, double lastX, double x,
	double lastAdvanceX, double scale, double lastY, double ty, double height,
	int selectionStart, int selectionEnd,
	const Color& selectionFG, const Color& underlineColor)
{
	double x2;
	if (lastY == ty) {
		x2 = x;
	} else {
		x2 = lastX + lastAdvanceX;
	}

	double offsetY = height / 6;

	if (selectionStart >= 0 && selectionEnd >= selectionStart
		&& (index - 1) >= selectionStart
		&& (index - 1) <= selectionEnd) {
		fRenderer.copy_bar(
			lastX / scale + xOffset, lastY + offsetY,
			x2 / scale + xOffset, lastY + offsetY, selectionFG);
	} else {
		fRenderer.copy_bar(
			lastX / scale + xOffset, lastY + offsetY,
			x2 / scale + xOffset, lastY + offsetY, underlineColor);
	}
}

