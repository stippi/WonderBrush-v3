/*
 * Copyright 2010, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved.
 */
#ifndef BRUSH_STROKE_H
#define BRUSH_STROKE_H

#include "BoundedObject.h"
#include "Brush.h"
#include "Listener.h"
#include "ObjectCache.h"
#include "Paint.h"

class StrokePoint {
public:
								StrokePoint();
								StrokePoint(const BPoint& point,
									float pressure, float tiltX, float tiltY);
								StrokePoint(const StrokePoint& other);

			StrokePoint&		operator=(const StrokePoint& other);
			bool				operator==(const StrokePoint& other) const;
			bool				operator!=(const StrokePoint& other) const;

			BPoint				point;
			float				pressure;
			float				tiltX;
			float				tiltY;
};

typedef ObjectCache<StrokePoint, false> Stroke;

class BrushStroke : public BoundedObject, public Listener {
public:
								BrushStroke();
	virtual						~BrushStroke();

	// Object interface
	virtual	ObjectSnapshot*		Snapshot() const;

	virtual	const char*			DefaultName() const;

	virtual	void				AddProperties(PropertyObject* object,
									uint32 flags = 0) const;
	virtual	bool				SetToPropertyObject(
									const PropertyObject* object,
									uint32 flags = 0);
	virtual	bool				HitTest(const BPoint& canvasPoint) const;

	// BoundedObject interface
	virtual	BRect				Bounds();

	// Listener interface
	virtual	void				ObjectChanged(const Notifier* object);

	// BrushStroke
			void				SetBrush(::Brush* brush);
	inline	::Brush*			Brush() const
									{ return fBrush.Get(); }

			void				SetPaint(::Paint* paint);
	inline	::Paint*			Paint() const
									{ return fPaint.Get(); }

	inline	const ::Stroke&		Stroke() const
									{ return fStroke; }
	inline	::Stroke&			Stroke()
									{ return fStroke; }

			bool				AppendPoint(const StrokePoint& point);

private:
			Reference< ::Brush>	fBrush;
			Reference< ::Paint>	fPaint;
			::Stroke			fStroke;
};

#endif // BRUSH_STROKE_H
