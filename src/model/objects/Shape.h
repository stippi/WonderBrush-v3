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
#include "Path.h"

class Shape : public Styleable {
public:
								Shape();
								Shape(const PathRef& path,
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

	virtual	bool				HitTest(const BPoint& canvasPoint);

	// Shape
			void				SetArea(const BRect& area);
			BRect				Area() const;

			void				SetPath(const PathRef& path);
			const PathRef&		GetPath() const;

	virtual	BRect				Bounds();

private:
			void				_GetPath(PathStorage& path) const;

private:
			BRect				fArea;
			PathRef				fPath;
			Path::Listener*		fPathListener;
};

#endif // SHAPE_H
