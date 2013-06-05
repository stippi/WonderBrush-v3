/*
 * Copyright 2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef COLOR_PROVIDER_H
#define COLOR_PROVIDER_H

#include <GraphicsDefs.h>

#include "BaseObject.h"

class ColorProvider : public BaseObject {
public:
								ColorProvider();
	virtual						~ColorProvider();

	virtual	rgb_color			GetColor() const = 0;
};

typedef Reference<ColorProvider> ColorProviderRef;

#endif // COLOR_PROVIDER_H
