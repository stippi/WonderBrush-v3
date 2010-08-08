/*
 * Copyright 2003-2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef BRUSH_H
#define BRUSH_H

#include "BaseObject.h"
#include "Transformable.h"

class Brush : public BaseObject {
public:
	// NOTE: Some of these flags are mutually exclusive,
	// for example, a solid brush ignores the hardness
	// completely.
	enum {
		FLAG_PRESSURE_CONTROLS_APHLA		= 0x01,
		FLAG_PRESSURE_CONTROLS_RADIUS		= 0x02,
		FLAG_PRESSURE_CONTROLS_HARDNESS		= 0x04,
		FLAG_PRESSURE_CONTROLS_SPACING		= 0x08,
		FLAG_SOLID							= 0x10,
		FLAG_TILT_CONTROLS_SHAPE			= 0x20,
	};

public:
								Brush(float minRadius, float maxRadius,
									float minHardness, float maxHardness);
	virtual						~Brush();

	// Brush
			void				SetRadius(float minRadius, float maxRadius);
	inline	float				MinRadius() const
									{ return fMinRadius; }
	inline	float				MaxRadius() const
									{ return fMaxRadius; }

			void				SetHardness(float minHardness, float maxHardness)
	inline	float				MinHardness() const
									{ return fMinHardness; }

			void				Draw(BPoint where, float pressure,
									 float tiltX, float tiltY,
									 float minAlpha, float maxAlpha, uint32 flags,
									 uint8* dest, uint32 bytesPerRow,
									 const Transformable& transform,
									 const BRect& constrainRect) const;

private:
			float				fMinRadius;
			float				fMaxRadius;
			float				fMinHardness;
			float				fMaxHardness;

	static	const uint8			sGaussTable[256];
};

#endif // BRUSH_H
