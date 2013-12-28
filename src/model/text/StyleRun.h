/*
 * Copyright 2012-2013 Stephan Aßmus <superstippi@gmx.de>
 * All rights reserved.
 */
#ifndef STYLE_RUN_H
#define STYLE_RUN_H

#include "CharacterStyle.h"

class CloneContext;

class StyleRun {
public:
								StyleRun(
									const CharacterStyleRef& characterStyle);
								StyleRun(const StyleRun& other);
								StyleRun(const StyleRun& other,
									CloneContext& context);

			StyleRun&			operator=(const StyleRun& other);
			bool				operator==(const StyleRun& other) const;
			bool				operator!=(const StyleRun& other) const;

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
