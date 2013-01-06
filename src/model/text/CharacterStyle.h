/*
 * Copyright 2012 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */
#ifndef CHARACTER_STYLE_H
#define CHARACTER_STYLE_H

#include "Font.h"
#include "Style.h"

class CharacterStyle : public Referenceable {
public:
									CharacterStyle(const Font& font,
										double glyphSpacing,
										double fauxWeight, double fauxItalic,
										const StyleRef& style);

									CharacterStyle(const CharacterStyle& other);

			bool					operator==(
										const CharacterStyle& other) const;

			bool					operator!=(
										const CharacterStyle& other) const;

	// Getters
	inline	const Font&				GetFont() const
										{ return fFont; }

	inline	double					GetGlyphSpacing() const
										 { return fGlyphSpacing; }
	inline	double					GetFauxWeight() const
										 { return fFauxWeight; }
	inline	double					GetFauxItalic() const
										 { return fFauxItalic; }

	inline	const StyleRef&			GetStyle() const
										{ return fStyle; }

	// Setters (return new instance with the changed property)
			CharacterStyle			SetFont(const Font& font) const;
			CharacterStyle			SetGlyphSpacing(double spacing) const;
			CharacterStyle			SetFauxWeight(double fauxWeight) const;
			CharacterStyle			SetFauxItalic(double fauxItalic) const;
			CharacterStyle			SetStyle(const StyleRef& style) const;


private:
			Font					fFont;
			double					fGlyphSpacing;
			double					fFauxWeight;
			double					fFauxItalic;
			StyleRef				fStyle;
};

typedef Reference<CharacterStyle>	CharacterStyleRef;

#endif // CHARACTER_STYLE_H
