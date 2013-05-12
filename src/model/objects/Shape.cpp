/*
 * Copyright 2007-2009, Stephan Aßmus <superstippi@gmx.de>.
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "Shape.h"

#include "ShapeSnapshot.h"

class PathListener : public Path::Listener {
public:
	PathListener(Shape* shape)
		: fShape(shape)
	{
	}

	virtual ~PathListener()
	{
	}

	virtual	void PointAdded(const Path* path, int32 index)
	{
		fShape->NotifyAndUpdate();
	}
	
	virtual	void PointRemoved(const Path* path, int32 index)
	{
		fShape->NotifyAndUpdate();
	}
	
	virtual	void PointChanged(const Path* path, int32 index)
	{
		fShape->NotifyAndUpdate();
	}
	
	virtual	void PathChanged(const Path* path)
	{
		fShape->NotifyAndUpdate();
	}
	
	virtual	void PathClosedChanged(const Path* path)
	{
		fShape->NotifyAndUpdate();
	}
	
	virtual	void PathReversed(const Path* path)
	{
		fShape->NotifyAndUpdate();
	}

private:
	Shape*	fShape;
};

// #pragma mark -

// constructor
Shape::Shape()
	: Styleable()
	, fPathListener(new(std::nothrow) PathListener(this))
{
	SetPath(PathRef(new(std::nothrow) Path(), true));
	InitBounds();
}

// constructor
Shape::Shape(const PathRef& path, const rgb_color& color)
	: Styleable(color)
	, fPathListener(new(std::nothrow) PathListener(this))
{
	SetPath(path);
	InitBounds();
}

// destructor
Shape::~Shape()
{
	printf("~Shape()\n");
	delete fPathListener;
}

// #pragma mark -

// Snapshot
ObjectSnapshot*
Shape::Snapshot() const
{
	return new ShapeSnapshot(this);
}

// AddProperties
void
Shape::AddProperties(PropertyObject* object, uint32 flags) const
{
	Styleable::AddProperties(object, flags);
}

// SetToPropertyObject
bool
Shape::SetToPropertyObject(const PropertyObject* object, uint32 flags)
{
	return Styleable::SetToPropertyObject(object, flags);
}

// DefaultName
const char*
Shape::DefaultName() const
{
	return "Shape";
}

// HitTest
bool
Shape::HitTest(const BPoint& canvasPoint)
{
	PathStorage path;
	_GetPath(path);
	RenderEngine engine(Transformation());
	return engine.HitTest(path, canvasPoint);
}

// #pragma mark -

// SetArea
void
Shape::SetArea(const BRect& area)
{
	if (area == fArea)
		return;

	BRect oldArea(fArea);
	fArea = area;

	UpdateChangeCounter();
	UpdateBounds();
}

// Area
BRect
Shape::Area() const
{
	return fArea;
}

// SetPath
void
Shape::SetPath(const PathRef& path)
{
	if (fPath == path)
		return;
	
	if (fPath.Get() != NULL && fPathListener != NULL)
		fPath->RemoveListener(fPathListener);

	fPath = path;

	if (fPath.Get() != NULL && fPathListener != NULL)
		fPath->AddListener(fPathListener);
}

// GetPath
const PathRef&
Shape::GetPath() const
{
	return fPath;
}

// Bounds
BRect
Shape::Bounds()
{
	BRect bounds;
	if (fPath.Get() != NULL) {
		bounds = fPath->Bounds();
		Style()->ExtendBounds(bounds);
	}
	return bounds;
}

// #pragma mark -

// _GetPath
void
Shape::_GetPath(PathStorage& path) const
{
	if (fPath.Get() == NULL)
		return;
	fPath->GetAGGPathStorage(path);
}

