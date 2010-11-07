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
								Brush();
								Brush(float minOpacity, float maxOpacity,
									float minRadius, float maxRadius,
									float minHardness, float maxHardness,
									uint32 flags);
								Brush(const Brush& other);
	virtual						~Brush();

	// BaseObject interface
	virtual	status_t			Unarchive(const BMessage* archive);
	virtual	status_t			Archive(BMessage* into,
									bool deep = true) const;
	virtual	void				AddProperties(PropertyObject* object,
									uint32 flags = 0) const;
	virtual	bool				SetToPropertyObject(
									const PropertyObject* object,
									uint32 flags = 0);
	virtual	const char*			DefaultName() const;

	// Brush
			void				SetMinOpacity(float opacity);
			void				SetMaxOpacity(float opacity);
			void				SetOpacity(float minOpacity, float maxOpacity);
	inline	float				MinOpacity() const
									{ return fMinOpacity; }
	inline	float				MaxOpacity() const
									{ return fMaxOpacity; }

			void				SetMinRadius(float radius);
			void				SetMaxRadius(float radius);
			void				SetRadius(float minRadius, float maxRadius);
	inline	float				MinRadius() const
									{ return fMinRadius; }
	inline	float				MaxRadius() const
									{ return fMaxRadius; }

			void				SetMinHardness(float hardness);
			void				SetMaxHardness(float hardness);
			void				SetHardness(float minHardness, float maxHardness);
	inline	float				MinHardness() const
									{ return fMinHardness; }

			void				SetFlags(uint32 flags, bool enable);
			void				SetFlags(uint32 flags);
	inline	uint32				Flags() const
									{ return fFlags; }

			uint8				Opacity(float pressure) const;
			float				Radius(float pressure) const;
			float				Hardness(float pressure) const;

			void				Draw(BPoint where, float pressure,
									float tiltX, float tiltY,
									uint8* dest, uint32 bytesPerRow,
									const Transformable& transform,
									const BRect& constrainRect) const;

private:
			float				fMinOpacity;
			float				fMaxOpacity;
			float				fMinRadius;
			float				fMaxRadius;
			float				fMinHardness;
			float				fMaxHardness;
			uint32				fFlags;
};

#endif // BRUSH_H
