/*
 * Copyright 2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef COLOR_H
#define COLOR_H

#include <GraphicsDefs.h>

#include "Notifier.h"

class Color : public Notifier {
public:
								Color();
								Color(const Color& color);
								Color(const rgb_color& color);
	virtual						~Color();

	inline						operator rgb_color() const
									{ return fColor; }

			Color&				SetColor(const rgb_color& color);

			Color&				operator=(const rgb_color& color);
			Color&				operator=(const Color& color);

			bool				operator==(const rgb_color& color) const;
			bool				operator!=(const rgb_color& color) const;

			bool				operator==(const Color& color) const;
			bool				operator!=(const Color& color) const;

private:
			rgb_color			fColor;
};

#endif // CURRENT_COLOR_H
