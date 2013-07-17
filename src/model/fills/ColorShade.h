/*
 * Copyright 2009-2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef COLOR_SHADE_H
#define COLOR_SHADE_H

#include "ColorProvider.h"
#include "Listener.h"

class ColorShade : public ColorProvider, public Listener {
public:
								ColorShade();
								ColorShade(const ColorShade& other);
								ColorShade(const ColorProviderRef& provider);
	virtual						~ColorShade();

	// ColorProvider interface
	virtual	rgb_color			GetColor() const;

	// BaseObject interface
	virtual	void				AddProperties(PropertyObject* object,
									uint32 flags = 0) const;
	virtual	bool				SetToPropertyObject(
									const PropertyObject* object,
									uint32 flags = 0);
	virtual	const char*			DefaultName() const;

	// Listener
	virtual void				ObjectChanged(const Notifier* object);

	// ColorShade
			ColorShade&			SetColorProvider(
									const ColorProviderRef& provider);

			ColorShade&			SetHue(float hue);
	inline	float				Hue() const
									{ return fHue; }

			ColorShade&			SetSaturation(float saturation);
	inline	float				Saturation() const
									{ return fSaturation; }

			ColorShade&			SetValue(float value);
	inline	float				Value() const
									{ return fValue; }

private:
			ColorProviderRef	fColorProvider;

			float				fHue;
			float				fSaturation;
			float				fValue;
};

#endif // COLOR_SHADE_H
