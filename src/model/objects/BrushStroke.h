/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef BRUSH_STROKE_H
#define BRUSH_STROKE_H

#include "BoundedObject.h"
#include "Brush.h"
#include "ObjectCache.h"

class StrokePoint {
public:
								StrokePoint();
								StrokePoint(const BPoint& where,
									float pressure, float tiltX, float tiltY);
								StrokePoint(const StrokePoint& other);

			StrokePoint&		operator=(const StrokePoint& other);
			bool				operator==(const StrokePoint& other) const;
			bool				operator!=(const StrokePoint& other) const;

			BPoint				where;
			float				pressure;
			float				tiltX;
			float				tiltY;
};

typedef ObjectCache<StrokePoint, false> Stroke;

class BrushStroke : public BoundedObject {
public:
								BrushStroke();
	virtual						~BrushStroke();

	// Object interface
	virtual	ObjectSnapshot*		Snapshot() const;

	virtual	const char*			DefaultName() const;

	virtual	bool				HitTest(const BPoint& canvasPoint) const;

	// BoundedObject interface
	virtual	BRect				Bounds();

	// BrushStroke
			void				SetBrush(::Brush* brush);
	inline	::Brush*			Brush() const
									{ return fBrush.Get(); }

	inline	const ::Stroke&		Stroke() const
									{ return fStroke; }
	inline	::Stroke&			Stroke()
									{ return fStroke; }

private:
			Reference< ::Brush>	fBrush;
			::Stroke			fStroke;
};

#endif // BRUSH_STROKE_H
