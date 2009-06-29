/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef PAINT_H
#define PAINT_H

#include <GraphicsDefs.h>

#include <agg_color_rgba.h>

#include "BaseObject.h"
#include "Listener.h"

class Gradient;

class Paint : public BaseObject, public Listener {
public:
	enum {
		NONE		= 0,
		COLOR		= 1,
		GRADIENT	= 2,
		PATTERN		= 3
	};

								Paint();
								Paint(const Paint& other);
								Paint(const rgb_color& color);
								Paint(const ::Gradient* gradient);
								Paint(BMessage* archive);

	virtual						~Paint();

	// BaseObject interface
	virtual	const char*			DefaultName() const;

	// Listener interface
	virtual	void				ObjectChanged(const Notifier* object);

	// Paint
	virtual	status_t			Archive(BMessage* into,
									bool deep = true) const;

			void				Unset();

			Paint&				operator=(const Paint& other);
			bool				operator==(const Paint& other) const;
			bool				operator!=(const Paint& other) const;
			Paint*				Clone() const;
			bool				HasTransparency() const;
			size_t				HashKey() const;

	inline	uint32				Type() const
									{ return fType; }

			void				SetColor(const rgb_color& color);
	inline	rgb_color			Color() const
									{ return (rgb_color&)fData.color; }

			void				SetGradient(const ::Gradient* gradient);
	inline	::Gradient*			Gradient() const
									{ return fData.gradient; }

	inline	const agg::rgba8*	Colors() const
									{ return fColors; }

//	inline	const agg::rgba8*	GammaCorrectedColors(
//									const GammaTable& table) const;

private:
			uint32				fType;
			union {
				uint32			color;
				::Gradient*		gradient;
			}					fData;

			// hold gradient color array
			agg::rgba8*			fColors;

			// for caching gamma corrected gradient color array
	mutable	agg::rgba8*			fGammaCorrectedColors;
	mutable	bool				fGammaCorrectedColorsValid;
};

#endif	// PAINT_H
