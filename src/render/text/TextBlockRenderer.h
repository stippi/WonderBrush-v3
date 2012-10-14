/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#ifndef TEXT_BLOCK_RENDERER_H
#define TEXT_BLOCK_RENDERER_H

#include <String.h>

#include <agg_conv_curve.h>
#include <agg_conv_contour.h>

#include "Font.h"
#include "FontCache.h"
#include "FontCacheEntry.h"
#include "List.h"
#include "Painter.h"

class TextBlockRenderer {
 private:
	struct word;
	struct line;

 public:
								TextBlockRenderer();
	virtual						~TextBlockRenderer();

	// configuring the TextBlockRenderer
			void				SetText(const char* utf8String);

			void				SetFont(const Font& font);

			void				SetLayout(float paragraphInset,
										  float paragraphSpacing,
										  float lineOffset,
										  float glyphSpacing,
										  float blockWidth,
										  uint8 alignment);

	// using the TextBlockRenderer
			void				RenderText(Painter& painter);

			BRect				Bounds(const AffineTransform& transform);

			BPoint				GetBaselinePosition(int32 glyphIndex);
			void				GetBaselinePositions(int32 startGlyphIndex,
									int32 endGlyphIndex, BPoint* _startOffset,
									BPoint* _endOffset);
			int32				GlyphIndexAt(BPoint baselinePosition);

			int32				FirstGlyphIndexAtLine(int32 currentGlyphIndex);
			int32				LastGlyphIndexAtLine(int32 currentGlyphIndex);
			int32				NextGlyphAtLineOffset(int32 currentGlyphIndex,
									int32 lineOffset);

			float				GlyphWidth(uint16 glyphIndex) const;

 private:
			float				_LineOffset() const;
			void				_Update();
			status_t			_PrepareUnicodeBuffer(const char* utf8String,
													  uint32 length,
													  uint32* glyphCount);
			void				_UpdateTransformation();
			bool				_IsWhiteSpace(uint16 glyph) const;

			template<class GlyphConsumer>
			void				_LayoutGlyphs(GlyphConsumer& consumer);
			void				_LayoutIfNeeded();
			int32				_FindLineAtOffset(float y) const;
			int32				_FindLineContaining(int32 glyphIndex) const;
			void				_GetGlyphPositionInfo(int32 glyphIndex,
									BPoint* _position, int32* _lineIndex,
									int32* _wordIndex);
			int32				_GlyphIndexAt(
									TextBlockRenderer::line* currentLine,
									BPoint position);


	FontCache*					fFontCache;

	Font						fFont;

	// Pipeline to process the vectors glyph paths (curves + contour)
	FontCacheEntry::GlyphPathAdapter	fPathAdaptor;
	FontCacheEntry::GlyphGray8Adapter	fGray8Adaptor;
	FontCacheEntry::GlyphGray8Scanline	fGray8Scanline;
	FontCacheEntry::GlyphMonoAdapter	fMonoAdaptor;
	FontCacheEntry::GlyphMonoScanline	fMonoScanline;

	FontCacheEntry::CurveConverter		fCurves;
	FontCacheEntry::ContourConverter	fContour;

	agg::scanline_u8			fScanline;

	char*						fUnicodeBuffer;
	int32						fUnicodeBufferSize;

	BString						fText;
	float						fParagraphInset;
	float						fParagraphSpacing;
	float						fLineOffset;
	float						fGlyphSpacing;
	float						fBlockWidth;
	uint8						fAlignment;
										  
	uint32						fGlyphCount;
	BRect						fBounds;
	bool						fNeedsLayout;

	List<line*>					fLines;

	float						fSpaceWidth;
};

#endif // TEXT_BLOCK_RENDERER_H
