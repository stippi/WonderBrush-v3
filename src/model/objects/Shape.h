/*
 * Copyright 2007-2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef SHAPE_H
#define SHAPE_H

#include <GraphicsDefs.h>
#include <List.h>
#include <Rect.h>

#include "RenderEngine.h"
#include "Styleable.h"

class Shape;

class ShapeListener {
public:
								ShapeListener();
	virtual						~ShapeListener();

	virtual	void				AreaChanged(Shape* shape,
									const BRect& oldArea,
									const BRect& newArea);
	virtual	void				Deleted(Shape* shape);
};


class Shape : public Styleable {
public:
								Shape();
								Shape(const BRect& area,
									const rgb_color& color);
	virtual						~Shape();

	// Object interface
	virtual	ObjectSnapshot*		Snapshot() const;

	virtual	void				AddProperties(PropertyObject* object,
									uint32 flags = 0) const;
	virtual	bool				SetToPropertyObject(
									const PropertyObject* object,
									uint32 flags = 0);

	virtual	const char*			DefaultName() const;

	virtual	bool				HitTest(const BPoint& canvasPoint) const;

	// Shape
			void				SetArea(const BRect& area);
			BRect				Area() const;
	virtual	BRect				Bounds();

			bool				AddListener(ShapeListener* listener);
			void				RemoveListener(ShapeListener* listener);

private:
			void				_NotifyAreaChanged(const BRect& oldArea,
									const BRect& newArea);

			void				_NotifyDeleted();

			void				_GetPath(PathStorage& path) const;

private:
			BRect				fArea;

			BList				fListeners;
};

#endif // SHAPE_H
