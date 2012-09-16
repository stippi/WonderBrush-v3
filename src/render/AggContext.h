/*
 * AggContext.h
 *
 *  Created on: 23.08.2012
 *      Author: stippi
 */

#ifndef AGGCONTEXT_H_
#define AGGCONTEXT_H_

#include "agg_conv_contour.h"
#include "agg_conv_curve.h"
#include "agg_conv_transform.h"
#include "agg_font_freetype.h"
#include "agg_gamma_lut.h"
#include "agg_path_storage.h"
#include "agg_pixfmt_rgb.h"
#include "agg_pixfmt_lcd_bgra.h"
#include "agg_pixfmt_lcd_argb.h"
#include "agg_pixfmt_rgba.h"
#include "agg_primary_weights.h"
#include "agg_scanline_bin.h"
#include "agg_scanline_u.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"

#include "FauxWeight.h"


typedef agg::rendering_buffer							RenderingBuffer;

typedef agg::rgba8										Color;

#ifdef __APPLE__
typedef agg::pixfmt_argb32_pre							PixelFormat;
#else
typedef agg::pixfmt_bgra32_pre							PixelFormat;
#endif
typedef agg::renderer_base<PixelFormat>					Renderer;
typedef agg::renderer_scanline_aa_solid<Renderer>		RendererSolid;

typedef agg::primary_weights							PrimaryWeights;
#ifdef __APPLE__
typedef agg::pixfmt_lcd_argb							PixelFormatLCD;
#else
typedef agg::pixfmt_lcd_bgra							PixelFormatLCD;
#endif
typedef agg::renderer_base<PixelFormatLCD>				RendererLCD;
typedef agg::renderer_scanline_aa_solid<RendererLCD>	RendererSolidLCD;

typedef agg::scanline32_u8								Scanline;
typedef agg::rasterizer_scanline_aa<>					Rasterizer;
typedef agg::path_storage								Path;

typedef agg::font_engine_freetype_int32					FontEngine;
typedef agg::font_cache_manager<FontEngine>				FontManager;

typedef agg::gamma_lut<>								GammaLUT;
typedef agg::trans_affine								Matrix;

typedef agg::conv_curve<FontManager::path_adaptor_type>	Glyph;
typedef agg::conv_transform<Glyph, Matrix>				TransformedGlyph;
typedef FauxWeight<TransformedGlyph>					FauxWeightGlyph;


class TextLayout;


class AggContext {
public:
	AggContext(int dpiX, int dpiY);

	void attachToBuffer(unsigned char* data, int width, int height, int stride);

	void setClipping(int x, int y, int width, int height);
	void unsetClipping();

	inline Path& getPath()
	{
		return fPath;
	}

	inline Rasterizer& getRasterizer()
	{
		return fRasterizer;
	}

	inline RendererSolid& getRendererSolid()
	{
		return fRendererSolid;
	}

	static inline FontEngine& getFontEngine()
	{
		return sFontEngine;
	}

	static inline FontManager& getFontManager()
	{
		return sFontManager;
	}

	inline int getWidth() const
	{
		return fBuffer.width();
	}

	inline int getHeight() const
	{
		return fBuffer.height();
	}

	void clear();
	void render();

	void setForeground(int red, int green, int blue, int alpha);
	void setBackground(int red, int green, int blue, int alpha);

	inline const Color& getForeground() const
	{
		return fForeground;
	}

	inline const Color& getBackground() const
	{
		return fBackground;
	}

	inline const bool getHinting() const
	{
		return fHinting;
	}

	inline const bool getKerning() const
	{
		return fKerning;
	}

	inline const bool getGrayScale() const
	{
		return fGrayScale;
	}

	bool loadFont(const char* fontFilePath, double height);

	double drawString(const char* text, double x, double y);

	double drawText(TextLayout* text, double x, double y,
		int selectionStart, int selectionEnd,
		const Color& selectionFG, const Color& selectionBG, unsigned flags);


private:
	template<class RendererType>
	double drawString(RendererType& renderer,
		const char* text, double x, double y, unsigned subpixelScale);

	template<class RendererType>
	double drawText(RendererType& renderer,
		TextLayout* layout, double x, double y,
		int selectionStart, int selectionEnd,
		const Color& selectionFG, const Color& selectionBG, unsigned flags,
		unsigned subpixelScale);

	void drawStrikeOut(int index, double xOffset, double lastX, double x,
		double lastAdvanceX, double scale, double lastY, double ty,
		double height, int selectionStart, int selectionEnd,
		const Color& selectionFG, const Color& strikeColor);

	void drawUnderline(int index, double xOffset, double lastX, double x,
		double lastAdvanceX, double scale, double lastY, double ty,
		double height, int selectionStart, int selectionEnd,
		const Color& selectionFG, const Color& underlineColor);

public:
	static const double		AUTO_HINT_SCALE = 100.0;

private:
	RenderingBuffer			fBuffer;
	PixelFormat				fPixelFormat;
	Renderer				fRenderer;
	RendererSolid			fRendererSolid;

	Color					fForeground;
	Color					fBackground;

	double					fGamma;
	double					fPrimaryWeight;
	double					fGlyphWidthScale;
	double					fGlyphSpacing;
	double					fFauxWeight;
	double					fFauxItalic;

	PrimaryWeights			fPrimaryWeights;
	PixelFormatLCD			fPixelFormatLCD;
	RendererLCD				fRendererLCD;
	RendererSolidLCD		fRendererSolidLCD;

	Scanline				fScanline;
	Rasterizer				fRasterizer;

	Path					fPath;

	static bool				sTextEngineInitialized;
	static FontEngine		sFontEngine;
	static FontManager		sFontManager;

	Matrix					fMatrix;

	// AGG-Pipeline to process vector glyphs (path->transformation->faux weight)
	Glyph					fGlyph;
	TransformedGlyph		fTransformedGlyph;
    FauxWeightGlyph			fFauxWeightGlyph;

	GammaLUT				fGammaLUT;

	bool					fHinting;
	bool					fKerning;
	bool					fGrayScale;
};


#endif /* AGGCONTEXT_H_ */
