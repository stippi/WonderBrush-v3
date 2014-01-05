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
	, fFillMode(FILL_MODE_NON_ZERO)
{
	InitBounds();
}

// constructor
Shape::Shape(const PathRef& path, const rgb_color& color)
	: Styleable(color)
	, fPathListener(new(std::nothrow) PathListener(this))
	, fFillMode(FILL_MODE_NON_ZERO)
{
	AddPath(path);
	InitBounds();
}

// constructor
Shape::Shape(const Shape& other, CloneContext& context)
	: Styleable(other, context)
	, fPathListener(new(std::nothrow) PathListener(this))
	, fFillMode(other.fFillMode)
{
	int32 count = other.Paths().CountItems();
	for (int32 i = 0; i < count; i++) {
		const PathRef& path = other.Paths().ItemAtFast(i);
		PathRef clonedPath;
		context.Clone(path.Get(), clonedPath);
		if (clonedPath.Get() != NULL)
			AddPath(clonedPath);
	}
	InitBounds();
}

// destructor
Shape::~Shape()
{
	int32 count = fPaths.CountItems();
	for (int32 i = 0; i < count; i++) {
		const PathRef& path = fPaths.ItemAtFast(i);
		path->RemoveListener(fPathListener);
	}
	delete fPathListener;
}

// #pragma mark -

// Clone
BaseObject*
Shape::Clone(CloneContext& context) const
{
	return new(std::nothrow) Shape(*this, context);
}

// DefaultName
const char*
Shape::DefaultName() const
{
	return "Shape";
}

// #pragma mark -

// Snapshot
ObjectSnapshot*
Shape::Snapshot() const
{
	return new ShapeSnapshot(this);
}

// Assets
AssetList
Shape::Assets() const
{
	AssetList list = Styleable::Assets();
	for (int32 i = 0; i < fPaths.CountItems(); i++) {
		const PathRef& ref = fPaths.ItemAtFast(i);
		list.Add(BaseObjectRef(ref.Get()));
	}
	return list;
}

// AddProperties
void
Shape::AddProperties(PropertyObject* object, uint32 flags) const
{
	Styleable::AddProperties(object, flags);
	// TODO: fFillMode
}

// SetToPropertyObject
bool
Shape::SetToPropertyObject(const PropertyObject* object, uint32 flags)
{
	// TODO: fFillMode
	return Styleable::SetToPropertyObject(object, flags);
}

// HitTest
bool
Shape::HitTest(const BPoint& canvasPoint)
{
	PathStorage path;
	GetPath(path);
	RenderEngine engine(Transformation());
	// TODO: fFillMode
	return engine.HitTest(path, canvasPoint);
}

// #pragma mark -

// SetFillMode
void
Shape::SetFillMode(uint32 fillMode)
{
	if (fFillMode != fillMode) {
		fFillMode = fillMode;
		NotifyAndUpdate();
	}
}

// AddPath
void
Shape::AddPath(const PathRef& path)
{
	if (path.Get() == NULL || !fPaths.Add(path))
		return;
	
	if (fPathListener != NULL)
		path->AddListener(fPathListener);

	NotifyAndUpdate();
}

// Paths
const PathList&
Shape::Paths() const
{
	return fPaths;
}

// Bounds
BRect
Shape::Bounds()
{
	BRect bounds(LONG_MAX, LONG_MAX, -LONG_MAX, -LONG_MAX);
	for (int32 i = fPaths.CountItems() - 1; i >= 0; i--) {
		const PathRef& path = fPaths.ItemAtFast(i);
		bounds = bounds | path->Bounds();
	}
	if (bounds.IsValid())
		Style()->ExtendBounds(bounds);
	return bounds;
}

// GetPath
void
Shape::GetPath(PathStorage& pathStorage) const
{
	for (int32 i = fPaths.CountItems() - 1; i >= 0; i--) {
		const PathRef& path = fPaths.ItemAtFast(i);
		path->GetAGGPathStorage(pathStorage);
	}
}

