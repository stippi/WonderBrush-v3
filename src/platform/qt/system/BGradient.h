/*
 * Copyright 2006-2009, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef PLATFORM_QT_B_GRADIENT_H
#define PLATFORM_QT_B_GRADIENT_H


#include <GraphicsDefs.h>


class BGradient {
public:
	struct ColorStop {
		ColorStop(const rgb_color c, float o);
		ColorStop(uint8 r, uint8 g, uint8 b, uint8 a, float o);
		ColorStop(const ColorStop& other);
		ColorStop();

		bool operator!=(const ColorStop& other) const;

		rgb_color		color;
		float			offset;
	};


	BGradient();
};


#endif // PLATFORM_QT_B_GRADIENT_H
