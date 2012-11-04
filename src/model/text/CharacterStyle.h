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
										const StyleRef& style);

	inline	const Font&				GetFont() const
										{ return fFont; }

	inline	const StyleRef&			GetStyle() const
										{ return fStyle; }

			bool					operator==(
										const CharacterStyle& other) const;

			bool					operator!=(
										const CharacterStyle& other) const;
private:
			Font					fFont;
			StyleRef				fStyle;
};

typedef Reference<CharacterStyle>	CharacterStyleRef;

#endif // CHARACTER_STYLE_H
