/*
 * Copyright 2006-2013, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef PAINT_H
#define PAINT_H

#include <GraphicsDefs.h>

#include <agg_color_rgba.h>

#include "ColorProvider.h"
#include "Listener.h"
#include "SharedObjectCache.h"

class Gradient;
class Paint;

typedef SharedObject<Paint>					SharedPaint;
typedef SharedObjectCache<Paint>			PaintCache;

class Paint : public BaseObject, public Listener {
public:
	enum {
		NONE			= 0,
		COLOR			= 1,
		GRADIENT		= 2,
		PATTERN			= 3
	};
	enum {
		FILL_PAINT		= 1 << 0,
		STROKE_PAINT	= 1 << 1,
	};

								Paint();
								Paint(const Paint& other);
								Paint(const Paint& other,
									CloneContext& context);
								Paint(const rgb_color& color);
								Paint(const ColorProviderRef& color);
								Paint(const ::Gradient* gradient);
								Paint(BMessage* archive);

	virtual						~Paint();

	// BaseObject interface
	virtual	BaseObject*			Clone(CloneContext& context) const;
	virtual	status_t			Unarchive(const BMessage* archive);
	virtual	status_t			Archive(BMessage* into,
									bool deep = true) const;
	virtual	void				AddProperties(PropertyObject* object,
									uint32 flags = 0) const;
	virtual	bool				SetToPropertyObject(
									const PropertyObject* object,
									uint32 flags = 0);
	virtual	const char*			DefaultName() const;

	// Listener interface
	virtual	void				ObjectChanged(const Notifier* object);

	// Paint
			Paint&				operator=(const Paint& other);
			bool				operator==(const Paint& other) const;
			bool				operator!=(const Paint& other) const;
			bool				HasTransparency() const;
			size_t				HashKey() const;

			void				SetType(uint32 type);
	inline	uint32				Type() const
									{ return fType; }

	static	void				AddTypeProperty(PropertyObject* object,
									uint32 propertyID, uint32 type);

			void				SetColor(const rgb_color& color);
	inline	rgb_color			Color() const
									{ return fColor->GetColor(); }
			void				SetColorProvider(
									const ColorProviderRef& colorProvider);
	inline	const ColorProviderRef& GetColorProvider() const
									{ return fColor; }

			void				SetGradient(const ::Gradient* gradient);
	inline	::Gradient*			Gradient() const
									{ return fGradient; }

	inline	const agg::rgba16*	Colors() const
									{ return fColors; }

//	inline	const agg::rgba8*	GammaCorrectedColors(
//									const GammaTable& table) const;

	static	const Paint&		EmptyPaint();
	static	::PaintCache&		PaintCache();

private:
			uint32				fType;
			ColorProviderRef	fColor;
			::Gradient*			fGradient;

			// hold gradient color array
			agg::rgba16*		fColors;
};

typedef Reference<Paint> PaintRef;

#endif	// PAINT_H
