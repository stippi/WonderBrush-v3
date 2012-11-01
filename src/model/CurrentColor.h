/*
 * Copyright 2006-2012, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef CURRENT_COLOR_H
#define CURRENT_COLOR_H

#include <GraphicsDefs.h>

#include "Notifier.h"

class CurrentColor : public Notifier {
public:
								CurrentColor();
	virtual						~CurrentColor();

			void				SetColor(const rgb_color& color);
	inline	const rgb_color&	Color() const
									{ return fColor; }

private:
			rgb_color			fColor;
};

#endif // CURRENT_COLOR_H
