/*
 * Copyright 2007-2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef SHAPE_H
#define SHAPE_H

#include <GraphicsDefs.h>
#include <List.h>
#include <Rect.h>

#include "List.h"
#include "RenderEngine.h"
#include "Styleable.h"
#include "Path.h"

typedef List<PathRef, false> PathList;

class Shape : public Styleable {
public:
								Shape();
								Shape(const PathRef& path,
									const rgb_color& color);
								Shape(const Shape& other,
									ResourceResolver& resolver);
	virtual						~Shape();

	// BaseObject interface
	virtual	BaseObject*			Clone(ResourceResolver& resolver) const;
	virtual	const char*			DefaultName() const;

	// Object interface
	virtual	ObjectSnapshot*		Snapshot() const;

	virtual	AssetList			Assets() const;

	virtual	void				AddProperties(PropertyObject* object,
									uint32 flags = 0) const;
	virtual	bool				SetToPropertyObject(
									const PropertyObject* object,
									uint32 flags = 0);

	virtual	bool				HitTest(const BPoint& canvasPoint);

	// Shape
			void				AddPath(const PathRef& path);
			const PathList&		Paths() const;

	virtual	BRect				Bounds();

			void				GetPath(PathStorage& path) const;

private:
			PathList			fPaths;
			Path::Listener*		fPathListener;
};

#endif // SHAPE_H
