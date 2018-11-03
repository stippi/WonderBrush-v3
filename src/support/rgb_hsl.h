/*
 * https://www.programmingalgorithms.com/algorithm/rgb-to-hsl?lang=C%2B%2B
 */
#ifndef RGB_HSL_H
#define RGB_HSL_H

#include <SupportDefs.h>

struct HSL {
public:
	HSL()
		: h(0.0f)
		, s(0.0f)
		, l(0.0f)
	{
	}

	float h;
	float s;
	float l;
};

static inline HSL
RGB_to_HSL(float r, float g, float b)
{
	HSL hsl;

	float min = std::min(std::min(r, g), b);
	float max = std::max(std::max(r, g), b);
	float delta = max - min;

	hsl.l = (max + min) / 2;

	if (delta == 0) {
		hsl.h = 0;
		hsl.s = 0.0f;
	} else {
		hsl.s = (hsl.l <= 0.5)
			? (delta / (max + min)) : (delta / (2 - max - min));

		float hue;

		if (r == max)
			hue = ((g - b) / 6) / delta;
		else if (g == max)
			hue = (1.0f / 3) + ((b - r) / 6) / delta;
		else
			hue = (2.0f / 3) + ((r - g) / 6) / delta;

		if (hue < 0)
			hue += 1;
		else if (hue > 1)
			hue -= 1;

		hsl.h = hue * 360.0f;
	}

	return hsl;
}

static inline float
Hue_to_RGB(float v1, float v2, float vH)
{
	if (vH < 0)
		vH += 1;

	if (vH > 1)
		vH -= 1;

	if ((6 * vH) < 1)
		return (v1 + (v2 - v1) * 6 * vH);

	if ((2 * vH) < 1)
		return v2;

	if ((3 * vH) < 2)
		return (v1 + (v2 - v1) * ((2.0f / 3) - vH) * 6);

	return v1;
}

static inline void
HSL_to_RGB(HSL hsl, uint8& r, uint8& g, uint8& b) {
	if (hsl.s == 0) {
		r = g = b = (unsigned char)(hsl.l * 255);
	} else {
		float v1, v2;
		float hue = (float)hsl.h / 360;

		v2 = (hsl.l < 0.5) ? (hsl.l * (1 + hsl.s))
			: ((hsl.l + hsl.s) - (hsl.l * hsl.s));
		v1 = 2 * hsl.l - v2;

		r = (uint8)(255 * Hue_to_RGB(v1, v2, hue + (1.0f / 3)));
		g = (uint8)(255 * Hue_to_RGB(v1, v2, hue));
		b = (uint8)(255 * Hue_to_RGB(v1, v2, hue - (1.0f / 3)));
	}
}

#endif // RGB_HSL_H
