/*
 * Copyright 2012 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */
#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include "agg_conv_contour.h"
#include "agg_conv_curve.h"
#include "agg_conv_transform.h"
#include "agg_font_freetype.h"
#include "agg_gamma_lut.h"
#include "agg_path_storage.h"
#include "agg_pixfmt_rgb.h"
#include "agg_pixfmt_lcd_bgra16.h"
#include "agg_pixfmt_rgba.h"
#include "agg_primary_weights.h"
#include "agg_scanline_bin.h"
#include "agg_scanline_u.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"

#include "FauxWeight.h"
#include "RenderEngine.h"


class FontCache;
class TextLayout;


class TextRenderer {
public:
	typedef agg::rendering_buffer							RenderingBuffer;

	typedef agg::rgba16										Color;

	typedef agg::scanline_u8								ScanlineUnpacked;

	typedef agg::renderer_base<PixelFormat>					Renderer;
	typedef agg::renderer_scanline_aa_solid<Renderer>		RendererSolid;

	typedef agg::primary_weights							PrimaryWeights;
	typedef agg::pixfmt_lcd_bgra16							PixelFormatLCD;
	typedef agg::renderer_base<PixelFormatLCD>				RendererLCD;
	typedef agg::renderer_scanline_aa_solid<RendererLCD>	RendererSolidLCD;

	typedef agg::path_storage								PathStorage;

	typedef agg::font_engine_freetype_int32					FontEngine;
	typedef agg::font_cache_manager<FontEngine>				FontManager;

	typedef agg::gamma_lut<>								GammaLUT;

	typedef agg::serialized_integer_path_adaptor<int32, 6>	PathAdaptor;
	typedef agg::conv_curve<PathAdaptor>					Glyph;
	typedef agg::conv_transform<Glyph, Transformation>		TransformedGlyph;
	typedef FauxWeight<TransformedGlyph>					FauxWeightGlyph;

public:
	TextRenderer(FontCache* fontCache);

	void attachToBuffer(unsigned char* data, int width, int height, int stride);

	void setClipping(int x, int y, int width, int height);
	void unsetClipping();

	inline PathStorage& getPath()
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

	FontEngine& getFontEngine() const;
	FontManager& getFontManager() const;

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

	void setHinting(bool hinting)
	{
		fHinting = hinting;
	}

	inline bool getHinting() const
	{
		return fHinting;
	}

	void setKerning(bool kerning)
	{
		fKerning = kerning;
	}

	inline bool getKerning() const
	{
		return fKerning;
	}

	void setGrayScale(bool grayScale)
	{
		fGrayScale = grayScale;
	}

	inline bool getGrayScale() const
	{
		return fGrayScale;
	}

	void setTransformation(const Transformation& transformation);

	bool loadFont(const char* fontFilePath, double height);

	double drawString(const char* text, double x, double y);

	double drawText(TextLayout* text, double x, double y,
		int selectionStart, int selectionEnd,
		const Color& selectionFG, const Color& selectionBG, unsigned flags);


private:
	void initPathAdaptor(const agg::glyph_cache* glyph, double x, double y,
		double scale = 1.0);

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

	ScanlineUnpacked		fScanline;
	Rasterizer				fRasterizer;

	PathStorage				fPath;

	FontCache*				fFontCache;

	Transformation			fBaseMatrix;
	Transformation			fMatrix;

	// AGG-Pipeline to process vector glyphs (path->transformation->faux weight)
	PathAdaptor				fPathAdaptor;
	Glyph					fGlyph;
	TransformedGlyph		fTransformedGlyph;
    FauxWeightGlyph			fFauxWeightGlyph;

	GammaLUT				fGammaLUT;

	bool					fHinting;
	bool					fKerning;
	bool					fGrayScale;
};


#endif // TEXT_RENDERER_H
