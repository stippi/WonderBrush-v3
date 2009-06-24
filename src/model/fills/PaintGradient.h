/*
 * Copyright 2006-2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef PAINT_GRADIENT_H
#define PAINT_GRADIENT_H


#include <agg_color_rgba.h>

#include "Listener.h"
#include "Paint.h"

class Gradient;

class PaintGradient : public Paint, public Listener {
public:
								PaintGradient();
								PaintGradient(const PaintGradient& other);
								PaintGradient(BMessage* archive);

	virtual						~PaintGradient();

	// Listener interface
	virtual	void				ObjectChanged(const Notifier* object);

	// Paint interface
	virtual	status_t			Archive(BMessage* into,
									bool deep = true) const;

	virtual	Paint*				Clone() const;
	virtual	bool				HasTransparency() const;
	virtual	uint32				Type() const
									{ return COLOR; }

	// PaintGradient
			void				SetGradient(const ::Gradient* gradient);
			::Gradient*			Gradient() const
									{ return fGradient; }

			const agg::rgba8*	Colors() const
									{ return fColors; }

//			const agg::rgba8*	GammaCorrectedColors(
//									const GammaTable& table) const;

private:
			::Gradient*			fGradient;

			// hold gradient color array
			agg::rgba8*			fColors;

			// for caching gamma corrected gradient color array
	mutable	agg::rgba8*			fGammaCorrectedColors;
	mutable	bool				fGammaCorrectedColorsValid;
};

#endif	// PAINT_GRADIENT_H
