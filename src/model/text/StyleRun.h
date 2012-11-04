/*
 * Copyright 2012 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */
#ifndef STYLE_RUN_H
#define STYLE_RUN_H

#include "CharacterStyle.h"

class StyleRun {
public:
								StyleRun(
									const CharacterStyleRef& characterStyle);
								StyleRun(const StyleRun& other);

	inline	const CharacterStyleRef& GetStyle() const
									{ return fCharacterStyle; }

	inline	int32				GetLength() const
									{ return fLength; }

			void				SetLength(int32 length);

			bool				IsSameStyle(const StyleRun& other) const;

private:
			CharacterStyleRef	fCharacterStyle;
			int32				fLength;
};

#endif // STYLE_RUN_H
