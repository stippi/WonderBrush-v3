/*
 * Copyright 2007, Haiku. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 */
#ifndef SHAPE_H
#define SHAPE_H

#include <GraphicsDefs.h>
#include <List.h>
#include <Rect.h>

#include "Object.h"

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


class Shape : public Object {
 public:
								Shape();
								Shape(const BRect& area,
									const rgb_color& color);
	virtual						~Shape();

	// Object interface
	virtual	ObjectSnapshot*		Snapshot() const;

	virtual	const char*			DefaultName() const;

	// Shape
			void				SetArea(const BRect& area);
			BRect				Area() const;

			void				SetColor(const rgb_color& color);
			rgb_color			Color() const;

			bool				AddListener(ShapeListener* listener);
			void				RemoveListener(ShapeListener* listener);

 private:
			void				_NotifyAreaChanged(const BRect& oldArea,
									const BRect& newArea);

			void				_NotifyDeleted();

			BRect				fArea;
			rgb_color			fColor;

			BList				fListeners;
};

#endif // SHAPE_H
