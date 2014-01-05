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
	enum FillMode {
		FILL_MODE_NON_ZERO = 0,
		FILL_MODE_EVEN_ODD,
	};

public:
								Shape();
								Shape(const PathRef& path,
									const rgb_color& color);
								Shape(const Shape& other,
									CloneContext& context);
	virtual						~Shape();

	// BaseObject interface
	virtual	BaseObject*			Clone(CloneContext& context) const;
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

			void				SetFillMode(uint32 fillMode);
	inline	uint32				FillMode() const
									{ return fFillMode; }

	virtual	BRect				Bounds();

			void				GetPath(PathStorage& path) const;

private:
			PathList			fPaths;
			Path::Listener*		fPathListener;
			uint32				fFillMode;
};

#endif // SHAPE_H
